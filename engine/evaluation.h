#ifndef eval_h
#define eval_h

#include "position.h"
#include <map>

namespace Medusa
{
	namespace Evaluation
	{
		const int PawnValue = 100;
		const int KnightValue = 300;
		const int BishopValue = 320;
		const int RookValue = 500;
		const int QueenValue = 950;
		const int KingValue = 9001;

		static const std::map<Piece, int> piece_values { {PAWN, PawnValue}, {KING, KingValue}, {KNIGHT, KnightValue}, {BISHOP, BishopValue}, {ROOK, RookValue}, {QUEEN, QueenValue} };

		// Used in the search.
		int StaticScore(Position &p);

		// Game phase
		enum GamePhase 
		{
			Opening,
			Middlegame,
			Endgame
		};

		// Colour evaluating (used for metaprogramming)
		enum EvalColour
		{
			White,
			Black
		};
		
		template<EvalColour C>
		Colour Get()
		{
			if (C == White)
				return Colour::WHITE;
			return Colour::BLACK;
		}
	};
};

#endif