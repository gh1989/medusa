#ifndef eval_h
#define eval_h

#include "parameters.h"
#include "position.h"

namespace medusa
{
	namespace eval
	{
		// Used in the search.
		int static_score(Position &p, const Parameters& params);

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
		int material_score(Position &p, const Parameters &params);

		// Pawn structure
		template<EvalColour C>
		int pawn_structure(Position &p,GamePhase phase, const Parameters &params);

		// King position
		template<EvalColour C>
		int king_position(Position &p,GamePhase phase, const Parameters &params);

		// Minor piece long-term potential.
		template<EvalColour C>
		int minor_pieces(Position &p,GamePhase phase, const Parameters &params);

		// Co-ordination
		template<EvalColour C>
		int coordination(Position &p,GamePhase phase, const Parameters &params);

		// Space
		template<EvalColour C>
		int space(Position &p,GamePhase phase, const Parameters &params);
		
		template <EvalColour C>
		int developedpcs(const Position &p);

		template<EvalColour C>
		int undevelopedpcs(const Position &p);

		template<EvalColour C>
		int unmovedpawns(const Position &p);
	};
};

#endif