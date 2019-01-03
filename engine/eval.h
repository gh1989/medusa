#pragma once

#include "position.h"

// Material
const int PAWN_SCORE   = 100;
const int KNIGHT_SCORE = 300;
const int BISHOP_SCORE = 300;
const int ROOK_SCORE   = 500;
const int QUEEN_SCORE  = 900;
const int KING_SCORE   = 10000000;

const int values[Piece::NUMBER_PIECES]
{
	KNIGHT_SCORE,
	BISHOP_SCORE,
	ROOK_SCORE,
	QUEEN_SCORE,
	KING_SCORE,
	PAWN_SCORE,
};

namespace Eval
{
	int static_score(Position &p);
};
