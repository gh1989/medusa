#pragma once

#include "position.h"
#include "move_deque.h"
#include "move_selector.h"
#include "Score.h"

#include <algorithm>
#include <memory>
#include <vector>
#include <list>
#include <sstream>
#include <string>
#include <thread>

class PositionSearcher
{
public:
	PositionSearcher():nodes_searched(0), first_leaf(false){}
	std::shared_ptr<MoveDeque> search_root(Position &pos, int max_depth);
	void search_thread(const Position &pos, int max_depth);
	void stop();

	static void set_searching_flag(bool value)
	{
		PositionSearcher::searching_flag = value;
	}

	static void reset_state()
	{
	}

	size_t get_nodes_searched() const { return nodes_searched; }

private:
	Score search(
		Position &pos, 
		std::shared_ptr<MoveDeque> md, 
		Score alpha,
		Score beta,
		int max_depth);

	Score search(
			Position &pos, 
			std::shared_ptr<MoveDeque> move_deque, 
			Score alpha, 
			Score beta, 
			int max_depth, 
			size_t plies_from_root);


	static void print_info(
			const std::shared_ptr<MoveDeque>& move_deque, 
			Score score, 
			size_t max_depth,
			size_t nodes,
			long long time);

	static size_t perft(Position &position, size_t depth);
	static bool searching_flag;
	std::shared_ptr<MoveDeque> principal_variation;
	size_t hard_max;
	std::thread thread;
	size_t nodes_searched;
	std::chrono::system_clock::time_point search_start_time;
	bool first_leaf;
};