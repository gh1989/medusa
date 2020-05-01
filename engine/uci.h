#pragma once

#include <deque>
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <atomic>
#include <shared_mutex>
#include <optional>

#include "search.h"
#include "utils.h"
#include "utils/mutex.h"

namespace Medusa {

	// Known commands.
	namespace {
		const std::unordered_map<std::string, std::unordered_set<std::string>>
			kKnownCommands = {
				{{"uci"}, {}},
				{{"selfplay"}, {}},
				{{"isready"}, {}},
				{{"setoption"}, {"context", "name", "value"}},
				{{"ucinewgame"}, {}},
				{{"position"}, {"fen", "startpos", "moves"}},
				{{"go"},
				 {"infinite", "wtime", "btime", "winc", "binc", "movestogo", "depth",
				  "nodes", "movetime", "searchmoves", "ponder"}},
				{{"start"}, {}},
				{{"stop"}, {}},
				{{"ponderhit"}, {}},
				{{"quit"}, {}},
				{{"xyzzy"}, {}},
		};
	}

	// Go parameters
	struct GoParams {
		optional<std::int64_t> wtime;
		optional<std::int64_t> btime;
		optional<std::int64_t> winc;
		optional<std::int64_t> binc;
		optional<int> movestogo;
		optional<int> depth;
		optional<int> nodes;
		optional<std::int64_t> movetime;
		bool infinite = false;
		std::vector<std::string> searchmoves;
		bool ponder = false;
	};

	// Current position
	struct CurrentPosition {
		std::string fen;
		std::vector<std::string> moves;
	};
		
	// Engine controller
	class EngineController {
	public:
		EngineController(
			BestMoveInfo::Callback best_move_callback,
			PvInfo::Callback info_callback );
		~EngineController() {
			// Make sure search is destructed first, and it still may be running in
			// a separate thread.
			search_.reset();
		}
		// Blocks.
		void EnsureReady();
		// Must not block.
		void NewGame();
		// Blocks.
		void SetPosition(const std::string& fen,
			const std::vector<std::string>& moves);
		// Must not block.
		void Go(const GoParams& params);
		// Must not block.
		void Stop();

	private:
		void SetupPosition(const std::string& fen,
			const std::vector<std::string>& moves);

		BestMoveInfo::Callback best_move_callback_;
		PvInfo::Callback info_callback_;

		// Locked means that there is some work to wait before responding readyok.
		RpSharedMutex busy_mutex_;
		using SharedLock = std::shared_lock<RpSharedMutex>;
		std::unique_ptr<Search> search_;
		Position current_position_instance_;
		optional<CurrentPosition> current_position_;
		GoParams go_params_;
		int64_t time_spared_ms_ = 0;
		std::chrono::steady_clock::time_point move_start_time_;
	};
	
	// UciLoop
	class UciLoop {
	public:
		UciLoop();
		~UciLoop() {}
		void RunLoop();

		// Sends response to host.
		void SendResponse(const std::string& response);
		// Sends responses to host ensuring they are received as a block.
		void SendResponses(const std::vector<std::string>& responses);
		void SendBestMove(const BestMoveInfo& move);
		void SendInfo(const std::vector<PvInfo>& infos);
		void SendId();

		// Command handlers.
		void CmdUci();
		void CmdIsReady();
		void CmdSetOption(const std::string&,
			const std::string&,
			const std::string&);
		void CmdUciNewGame();
		void CmdPosition(const std::string&,
			const std::vector<std::string>&);
		void CmdGo(const GoParams&);
		void CmdStop();

	private:
		bool DispatchCommand(
			const std::string& command,
			const std::unordered_map<std::string, std::string>& params);
		EngineController engine_;
		
	};
}

