#ifndef MOVE_SELECTOR_H
#define MOVE_SELECTOR_H

#include <map>
#include "board.h"

namespace medusa
{
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
};

#endif