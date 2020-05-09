#pragma once
#include <map>
#include "evaluation.h"
#include "position.h"

namespace Medusa
{
	class MoveSelector
	{

	public:
		MoveSelector(Position &pos, bool include_quiet);
		bool Any() const { return !moves.empty(); }
		auto GetMoves() const { return moves; }
		size_t NumMoves() const { return moves.size(); }
		int SEE(Position &pos, Move move);

	private:
		std::multimap<int, Move> moves;
	};
};