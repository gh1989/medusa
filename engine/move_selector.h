#ifndef MOVE_SELECTOR_H
#define MOVE_SELECTOR_H

#include <map>
#include "move_tiny.h"

const int CHCK_PRI = 1000.0;
const int CAPT_PRI = 10000.0;

class Position;
class Move;

class MoveSelector
{
public:
	MoveSelector(Position &pos, bool include_quiet);
	bool any() const { return !moves.empty(); }
	auto get_moves() const { return moves; }
	size_t num_moves() const { return moves.size(); }
	int see(Position &pos, MoveTiny move);

private:
	std::multimap<int, MoveTiny> moves;
};

#endif