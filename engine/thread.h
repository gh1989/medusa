#pragma once

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

#include "position.h"
#include "utils/mutex.h"

namespace Medusa
{

	class Thread
	{
	public:
		~Thread() {
			Abort();
			Wait();
		}

		bool IsSearchActive() const {
			return !stop_.load(std::memory_order_acquire);
		}

		void FireStopInternal() {
			stop_.store(true, std::memory_order_release);
			watchdog_cv.notify_all();
		}

		void Stop() {
			Mutex::Lock lock(counters_mutex);
			FireStopInternal();
		}

		void Abort() {
			Mutex::Lock lock(counters_mutex);
			if (!stop_.load(std::memory_order_acquire)) {
				FireStopInternal();
			}
		}

		void Wait() {
			Mutex::Lock lock(threads_mutex);
			while (!threads.empty()) {
				threads.back().join();
				threads.pop_back();
			}
		}

		void StartThread(
			Position &pos,
			int max_depth,
			BestMoveInfo::Callback bestcallback,
			PvInfo::Callback infocallback);

		void Start(const Position &pos, int max_depth);


	private:
		std::thread thread;
		mutable Mutex counters_mutex;
		std::vector<std::thread> threads GUARDED_BY(threads_mutex);
		std::condition_variable watchdog_cv;
		Mutex threads_mutex;
		std::atomic<bool> stop_{ false };
	};
};