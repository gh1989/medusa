#pragma once

#include "move.h"
#include "position.h"

const int AS_SCORE = 2;			// Attacked square 
const int CO_SCORE = 2;  		// Central occupation per square
const int AO_SCORE = 2;			// Advanced occupation per square 
const int KS_SCORE = 2;			// King square attacked 

// Material
const int PAWN_SCORE = 100;
const int KNIGHT_SCORE = 300;
const int BISHOP_SCORE = 300;
const int ROOK_SCORE = 500;
const int QUEEN_SCORE = 900;

// Priorities
const int PROMISE_CENTRAL = 100;
const int PROMISE_CAPTURE = 1000;
const int PROMISE_CHECK   = 100000;

const int values[Piece::NUMBER_PIECES]
{
	PAWN_SCORE,
	KNIGHT_SCORE,
	BISHOP_SCORE,
	ROOK_SCORE,
	QUEEN_SCORE
};

namespace Eval
{
	int static_score(Position &p);
	int move_promise(Position &p, Move move);
};
