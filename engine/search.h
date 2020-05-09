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

		Score _Search(
			Position &pos,
			std::shared_ptr<Variation> md,
			Score alpha,
			Score beta,
			int max_depth);

		Score QSearch(
			Position &position,
			Score alpha,
			Score beta,
			std::shared_ptr<Variation> vrtn,
			int dfr);

	private:


		static size_t Perft(Position &position, size_t depth);
		static bool searching_flag;
		std::shared_ptr<Variation> principal_variation;
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
	};
}

#endif