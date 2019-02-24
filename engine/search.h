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

namespace medusa 
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
			PvInfo::Callback info_callback)
		{
			searching_flag = true;
			threads.emplace_back(
				[this, pos, max_depth, bestmove_callback, info_callback]
			{
				auto cpos = pos;
				search_root(cpos, max_depth);
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

		std::shared_ptr<Variation> search_root(Position &pos, int max_depth);
		void start(const Position &pos, int max_depth);
		void stop() { searching_flag = false; }
		static void set_searching_flag(bool value)
		{
			Search::searching_flag = value;
		}

		size_t get_nodes_searched() const { return nodes_searched; }

	private:
		Score search(
			Position &pos,
			std::shared_ptr<Variation> md,
			Score alpha,
			Score beta,
			int max_depth);

		Score search(
			Position &pos,
			std::shared_ptr<Variation> move_deque,
			Score alpha,
			Score beta,
			int max_depth,
			size_t plies_from_root);

		static size_t perft(Position &position, size_t depth);
		static bool searching_flag;
		std::shared_ptr<Variation> principal_variation;
		size_t hard_max;
		std::thread thread;
		size_t nodes_searched;
		std::chrono::system_clock::time_point search_start_time;
		bool first_leaf;
		mutable Mutex counters_mutex;
		bool bestmove_is_sent GUARDED_BY(counters_mutex) = false;
		std::vector<std::thread> threads GUARDED_BY(threads_mutex);
		bool ok_to_respond_bestmove GUARDED_BY(counters_mutex) = true;
		std::condition_variable watchdog_cv;
		Mutex threads_mutex;
		std::atomic<bool> stop_{ false };
		BestMoveInfo best_move_info;
};

	const int CHCK_PRI = 1000.0;
	const int CAPT_PRI = 10000.0;

	class MoveSelector
	{
	public:
		MoveSelector(Position &pos, bool include_quiet);
		bool any() const { return !moves.empty(); }
		auto get_moves() const { return moves; }
		size_t num_moves() const { return moves.size(); }
		int see(Position &pos, Move move);

	private:
		std::multimap<int, Move> moves;
	};
}

#endif