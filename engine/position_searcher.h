#pragma once

#include "Move.h"
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

class PositionSearcher
{
public:
	std::shared_ptr<MoveDeque> search(Position &pos, int max_depth);

	static void set_searching_flag(bool value)
	{
		PositionSearcher::searching_flag = value;
	}

	static void reset_state()
	{
	}

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
			size_t max_depth);

	static bool searching_flag;
	std::shared_ptr<MoveDeque> principal_variation;
	size_t hard_max;
};