#include "uci.h"
#include "types.h"
#include "utils.h"
#include "utils/logging.h"

#include <exception>
#include <functional>
#include <iomanip>
#include <iostream>
#include <ios>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>


namespace medusa {

	std::string GetOrEmpty(
		const std::unordered_map<std::string, std::string>& params,
		const std::string& key) {
		auto iter = params.find(key);
		if (iter == params.end()) return {};
		return iter->second;
	}

	int GetNumeric(const std::unordered_map<std::string, std::string>& params,
		const std::string& key) {
		auto iter = params.find(key);
		if (iter == params.end()) {
			throw Exception("Unexpected error");
		}
		const std::string& str = iter->second;
		try {
			if (str.empty()) {
				throw Exception("expected value after " + key);
			}
			return std::stoi(str);
		}
		catch (std::invalid_argument& e) {
			throw Exception("invalid value " + str);
		}
	}

	bool ContainsKey(const std::unordered_map<std::string, std::string>& params,
		const std::string& key) {
		return params.find(key) != params.end();
	}

	std::pair<std::string, std::unordered_map<std::string, std::string>>
		ParseCommand(const std::string& line) {
		std::unordered_map<std::string, std::string> params;
		std::string* value = nullptr;

		std::istringstream iss(line);
		std::string token;
		iss >> token >> std::ws;

		// If empty line, return empty command.
		if (token.empty()) return {};

		auto command = kKnownCommands.find(token);
		if (command == kKnownCommands.end()) {
			throw Exception("Unknown command: " + line);
		}

		std::string whitespace;
		while (iss >> token) {
			auto iter = command->second.find(token);
			if (iter == command->second.end()) {
				if (!value) throw Exception("Unexpected token: " + token);
				*value += whitespace + token;
				whitespace = " ";
			}
			else {
				value = &params[token];
				iss >> std::ws;
				whitespace = "";
			}
		}
		return { command->first, params };
	}

	UciLoop::UciLoop()
		: engine_(
			std::bind(&UciLoop::SendBestMove, this, std::placeholders::_1), 
			std::bind(&UciLoop::SendInfo, this, std::placeholders::_1)) 
	{
	}

	void UciLoop::RunLoop()
	{
		std::cout.setf(std::ios::unitbuf);
		std::string line;
		while (std::getline(std::cin, line)) {
			try {
				LOGFILE << ">>>" << line;
				auto command = ParseCommand(line);
				// Ignore empty line.
				if (command.first.empty()) continue;
				if (!DispatchCommand(command.first, command.second)) break;
			}
			catch (Exception& ex) {
				SendResponse(std::string("error ") + ex.what());
			}
		}
	}

	void UciLoop::SendResponse(const std::string& response)
	{
		SendResponses({ response });
	}

	void UciLoop::SendResponses(const std::vector<std::string>& responses)
	{
		static std::mutex output_mutex;
		std::lock_guard<std::mutex> lock(output_mutex);
		for (auto& response : responses)
		{
			std::cout << response << std::endl;
			LOGFILE << response;
		}
	}

	void UciLoop::SendBestMove(const BestMoveInfo& move)
	{
		std::string res = "bestmove " + as_uci(move.best_move);
		SendResponse(res);
	}

	void UciLoop::SendInfo(const std::vector<PvInfo>& infos)
	{
		std::vector<std::string> reses;
		for (const auto& info : infos) {
			std::string res = "info";
			if (info.depth >= 0) res += " depth " + std::to_string(info.depth);
			if (info.time >= 0) res += " time " + std::to_string(info.time);
			if (info.nodes >= 0) res += " nodes " + std::to_string(info.nodes);
			
			if (!info.score.is_mate())
				res += " score cp " + std::to_string(int(info.score.get_centipawns()));
			else
				res += " score mate " + std::to_string(info.score.get_mate_in());
			if (info.nps >= 0) res += " nps " + std::to_string(info.nps);

			res += " pv " + get_line(info.pv);
			reses.push_back(std::move(res));
		}
		SendResponses(reses);
	}

	void UciLoop::SendId() {
		SendResponse("id name Medusa");
		SendResponse("id author Gregg Hutchence");
	}

	void UciLoop::CmdUci()
	{
		SendResponse("uciok");
	}

	void UciLoop::CmdIsReady()
	{
		engine_.EnsureReady();
		SendResponse("readyok");
	}

	void UciLoop::CmdUciNewGame()
	{
		engine_.NewGame();
	}

	void UciLoop::CmdPosition(const std::string& position,
		const std::vector<std::string>& moves)
	{
		std::string fen = position;
		if (fen.empty()) fen = medusa::start_pos_fen;
		engine_.SetPosition(fen, moves);
	}

	void UciLoop::CmdGo(const GoParams& params)
	{
		engine_.Go(params);
	}

	void UciLoop::CmdStop()
	{
		engine_.Stop();
	}

	bool UciLoop::DispatchCommand(
		const std::string& command,
		const std::unordered_map<std::string, std::string>& params)
	{
		if (command == "uci")
		{
			CmdUci();
		}
		else if (command == "isready")
		{
			CmdIsReady();
		}
		else if (command == "ucinewgame")
		{
			CmdUciNewGame();
		}
		else if (command == "position")
		{
			if (ContainsKey(params, "fen") == ContainsKey(params, "startpos")) {
				throw Exception("Position requires either fen or startpos");
			}
			std::vector<std::string> moves =
				StrSplitAtWhitespace(GetOrEmpty(params, "moves"));
			CmdPosition(GetOrEmpty(params, "fen"), moves);
		}
		else if (command == "go")
		{
			GoParams go_params;
			if (ContainsKey(params, "infinite"))
			{
				if (!GetOrEmpty(params, "infinite").empty()) {
					throw Exception("Unexpected token " + GetOrEmpty(params, "infinite"));
				}
				go_params.infinite = true;
			}
			if (ContainsKey(params, "searchmoves"))
			{
				go_params.searchmoves =
					StrSplitAtWhitespace(GetOrEmpty(params, "searchmoves"));
			}
			if (ContainsKey(params, "ponder"))
			{
				if (!GetOrEmpty(params, "ponder").empty()) {
					throw Exception("Unexpected token " + GetOrEmpty(params, "ponder"));
				}
				go_params.ponder = true;
			}

#define UCIGOOPTION(x)                \
			if (ContainsKey(params, #x)) {        \
			go_params.x = GetNumeric(params, #x); \
			}
			UCIGOOPTION(wtime);
			UCIGOOPTION(btime);
			UCIGOOPTION(winc);
			UCIGOOPTION(binc);
			UCIGOOPTION(movestogo);
			UCIGOOPTION(depth);
			UCIGOOPTION(nodes);
			UCIGOOPTION(movetime);

#undef UCIGOOPTION
			CmdGo(go_params);
		}
		else if (command == "stop")
		{
			CmdStop();
		}
		else if (command == "ponderhit")
		{
			SendResponse("Nothing happens.");
		}
		else if (command == "start")
		{
			SendResponse("Nothing happens.");
		}
		else if (command == "xyzzy")
		{
			SendResponse("Nothing happens.");
		}
		else if (command == "quit")
		{
			return false;
		}
		else {
			throw Exception("Unknown command: " + command);
		}
		return true;
	}

	EngineController::EngineController(
		BestMoveInfo::Callback best_move_callback,
		PvInfo::Callback info_callback)
		: best_move_callback_(best_move_callback), info_callback_(info_callback)
	{
	}
	
	// Blocks.
	void EngineController::EnsureReady()
	{
		std::unique_lock<RpSharedMutex> lock(busy_mutex_);
		// If a UCI host is waiting for our ready response, we can consider the move
		// not started until we're done ensuring ready.
		move_start_time_ = std::chrono::steady_clock::now();
	}

	// Must not block.
	void EngineController::NewGame()
	{
		// In case anything relies upon defaulting to default position and just calls
		// newgame and goes straight into go.
		move_start_time_ = std::chrono::steady_clock::now();
		SharedLock lock(busy_mutex_);
		search_.reset();
		time_spared_ms_ = 0;
		current_position_.reset();
	}

	// Blocks.
	void EngineController::SetPosition(const std::string& fen,
		const std::vector<std::string>& moves_str)
	{
		// Some UCI hosts just call position then immediately call go, while starting
		// the clock on calling 'position'.
		move_start_time_ = std::chrono::steady_clock::now();
		SharedLock lock(busy_mutex_);
		current_position_ = CurrentPosition{ fen, moves_str };
		search_.reset();
	}

	// Must not block.
	void EngineController::Go(const GoParams& params)
	{
		auto start_time = move_start_time_;


		// Given wtime, btime, winc, binc manage the time. Simple model for this
		// is to estimate the number of moves left in the game, then expected total
		// time is expected_moves * [w/b]inc + [w/b]time and time per move should be
		// that divided by expected_moves. If time ever goes below 20 seconds then we
		// should use [w/b]inc. 
		go_params_ = params;
		auto us = current_position_instance_.colour_to_move();
		int ctime = us.is_black() ? go_params_.btime.value_or(0) : go_params_.wtime.value_or(0);
		int cinc  = us.is_black() ? go_params_.binc.value_or(0) : go_params_.winc.value_or(0);
		int emoves = std::max((100 - current_position_instance_.get_plies())/2, 10);
		int etime = ctime + emoves * cinc;
		int stime = std::max(etime / emoves, 1000);
		LOGFILE << "Time per move (ms): " << stime << std::endl;

		PvInfo::Callback info_callback(info_callback_);
		BestMoveInfo::Callback best_move_callback(best_move_callback_);

		// Setting up current position, now that it's known whether it's ponder or
		// not.
		if (current_position_)
				SetupPosition(current_position_->fen, current_position_->moves);
		else
			SetupPosition(medusa::start_pos_fen, {});

		search_ = std::make_unique<Search>();
		search_->StartThread(
				current_position_instance_, 
				go_params_.depth.value_or(6),
				best_move_callback_,
				info_callback_,
				stime);
	}

	// Must not block.
	void EngineController::Stop()
	{
		if (search_)
			search_->stop();
	}

	// Set up position.
	void EngineController::SetupPosition(const std::string& fen,
		const std::vector<std::string>& moves_str)
	{
		SharedLock lock(busy_mutex_);
		search_.reset();
		
		current_position_instance_ = position_from_fen(fen);
		std::vector<Move> moves;
		for (const auto& move : moves_str) 
			current_position_instance_.apply_uci(move);
	}
}
