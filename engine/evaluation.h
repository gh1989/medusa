#ifndef eval_h
#define eval_h

#include "position.h"

namespace medusa
{
		
	// Material
	const int PAWN_SCORE = 100;
	const int KNIGHT_SCORE = 300;
	const int BISHOP_SCORE = 300;
	const int ROOK_SCORE = 500;
	const int QUEEN_SCORE = 900;
	const int KING_SCORE = 10000000;

	const int values[Piece::NUMBER_PIECES]
	{
		KNIGHT_SCORE,
		BISHOP_SCORE,
		ROOK_SCORE,
		QUEEN_SCORE,
		KING_SCORE,
		PAWN_SCORE,
	};

	namespace eval
	{
		// Used in the search.
		int static_score(Position &p);

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
		Colour get()
		{
			if (C == White)
				return Colour::WHITE;
			return Colour::BLACK;
		}

		// Game phase
		template<EvalColour C>
		GamePhase game_phase(const Position &p);

		// Material
		template<EvalColour C>
		int material_score(Position &p);

		// Pawn structure
		template<EvalColour C>
		int pawn_structure(Position &p, GamePhase phase);

		// King position
		template<EvalColour C>
		int king_position(Position &p, GamePhase phase);

		// Minor piece long-term potential.
		template<EvalColour C>
		int minor_pieces(Position &p, GamePhase phase);

		// Co-ordination
		template<EvalColour C>
		int coordination(Position &p, GamePhase phase);

		// Space
		template<EvalColour C>
		int space(Position &p, GamePhase phase);
		
		template <EvalColour C>
		int developedpcs(const Position &p);

		template<EvalColour C>
		int undevelopedpcs(const Position &p);

		template<EvalColour C>
		int unmovedpawns(const Position &p);
	};
};

#endif