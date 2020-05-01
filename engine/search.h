#ifndef position_searcher_h
#define position_searcher_h

#ifndef _SILENCE_PARALLEL_ALGORITHMS_EXPERIMENTAL_WARNING
#define _SILENCE_PARALLEL_ALGORITHMS_EXPERIMENTAL_WARNING
#endif

#include <algorithm>
#include <map>
#include <memory>
#include <vector>
#include <list>
#include <sstream>
#include <string>
#include <thread>
#include "utils/mutex.h"

#include "position.h"
#include "move.h"
#include "utils.h"
#include "evaluation.h"
#include "parameters.h"

namespace Medusa 
{
	class Search
	{
	public:
		Search() :nodes_searched(0), first_leaf(false), bestmove_is_sent(false) {}
		~Search() {
			Abort();
			Wait();
		}

		void StartThread(
			Position &pos,
			int max_depth,
			BestMoveInfo::Callback bestmove_callback,
			PvInfo::Callback info_callback_,
			int time_limit)
		{
			searching_flag = true;
			info_callback = info_callback_;
			search_start_time = std::chrono::system_clock::now();
			search_time_limit = std::max(time_limit, 200);
			
			threads.emplace_back(
				[this, pos, max_depth, bestmove_callback]
			{
				auto cpos = pos;
				SearchRoot(cpos, max_depth);
				bestmove_callback(best_move_info);
				Mutex::Lock lock(counters_mutex);
				bestmove_is_sent = true;
			});
		}	

		bool IsSearchActive() const
		{
			return !stop_.load(std::memory_order_acquire);
		}

		void FireStopInternal()
		{
			stop_.store(true, std::memory_order_release);
			watchdog_cv.notify_all();
		}

		void Stop()
		{
			Mutex::Lock lock(counters_mutex);
			ok_to_respond_bestmove = true;
			FireStopInternal();
		}

		void Abort()
		{
			Mutex::Lock lock(counters_mutex);
			if (!stop_.load(std::memory_order_acquire)) {
				bestmove_is_sent = true;
				FireStopInternal();
			}
		}

		void Wait()
		{
			Mutex::Lock lock(threads_mutex);
			while (!threads.empty()) {
				threads.back().join();
				threads.pop_back();
			}
		}

		std::shared_ptr<Variation> SearchRoot(Position &pos, int max_depth);
		void Start(const Position &pos, int max_depth);
		
		static void set_searching_flag(bool value)
		{
			Search::searching_flag = value;
		}

		size_t GetNodesSearched() const { return nodes_searched; }

	private:
		Score _Search(
			Position &pos,
			std::shared_ptr<Variation> md,
			Score alpha,
			Score beta,
			int max_depth);

		Score _Search(
			Position &pos,
			std::shared_ptr<Variation> move_deque,
			Score alpha,
			Score beta,
			int max_depth,
			size_t plies_from_root);

		static size_t Perft(Position &position, size_t depth);
		static bool searching_flag;
		std::shared_ptr<Variation> principal_variation;
		size_t hard_max;
		std::thread thread;
		size_t nodes_searched;
		std::chrono::system_clock::time_point search_start_time;
		int search_time_limit;
		bool first_leaf;
		mutable Mutex counters_mutex;
		bool bestmove_is_sent GUARDED_BY(counters_mutex) = false;
		std::vector<std::thread> threads GUARDED_BY(threads_mutex);
		bool ok_to_respond_bestmove GUARDED_BY(counters_mutex) = true;
		std::condition_variable watchdog_cv;
		Mutex threads_mutex;
		std::atomic<bool> stop_{ false };
		BestMoveInfo best_move_info;
		PvInfo::Callback info_callback;
		Parameters params;
};

	const int CHCK_PRI = 1000.0;
	const int CAPT_PRI = 10000.0;

	class MoveSelector
	{
	public:
		MoveSelector(Position &pos, bool include_quiet, const Parameters &params_);
		bool Any() const { return !moves.empty(); }
		auto GetMoves() const { return moves; }
		size_t NumMoves() const { return moves.size(); }
		int SEE(Position &pos, Move move);

	private:
		std::multimap<int, Move> moves;
		Parameters params;
	};
	   
	inline int MoveSelector::SEE(Position &pos, Move move)
	{
		int value = 0;

		// Get smallest attacker capture
		auto next_moves = pos.LegalMoves<Capture>();
		auto remove_condition = [pos](Move m) { return !pos.MoveIsCapture(m); };
		auto to_remove = std::remove_if(next_moves.begin(), next_moves.end(), remove_condition);
		next_moves.erase(to_remove, next_moves.end());

		bool capture_exists = next_moves.size();
		if (!capture_exists)
		{
			return value;
		}

		// Get the smallest attacker
		auto next_move = next_moves.back();
		int smallest_attacker = 100000;
		for (auto m : next_moves)
		{
			int attacker = params.Get( ParameterID( pos.GetAttacker(next_move) ) );
			if (attacker <= smallest_attacker)
			{
				smallest_attacker = attacker;
				next_move = m;
			}
		}

		// === Apply ====
		auto captured = pos.Captured(next_move);
		pos.Apply(next_move);
		int capture_value = params.Get(ParameterID(captured)) - SEE(pos, next_move);
		pos.Unapply(next_move);
		// === unapply ====

		value = capture_value > 0 ? capture_value : 0;
		return value;
	}
}

#endif