#ifndef eval_h
#define eval_h

#include "parameters.h"
#include "position.h"

namespace Medusa
{
	namespace Evaluation
	{
		// Used in the search.
		int StaticScore(Position &p, const Parameters& params);

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

		// Game phase
		template<EvalColour C>
		GamePhase CalcGamePhase(const Position &p);

		// Material
		template<EvalColour C>
		int CalcMaterialScore(Position &p, const Parameters &params);

		// Pawn structure
		template<EvalColour C>
		int CalcPawnStructureScore(Position &p,GamePhase phase, const Parameters &params);

		// King position
		template<EvalColour C>
		int CalcKingPositionScore(Position &p,GamePhase phase, const Parameters &params);

		// Minor piece long-term potential.
		template<EvalColour C>
		int CalcMinorPiecePotentialScore(Position &p,GamePhase phase, const Parameters &params);

		// Co-ordination
		template<EvalColour C>
		int CalcCoordinationScore(Position &p,GamePhase phase, const Parameters &params);

		// Space
		template<EvalColour C>
		int CalcSpaceScore(Position &p,GamePhase phase, const Parameters &params);
		
		template <EvalColour C>
		int CalcDevelopedPieces(const Position &p);

		template<EvalColour C>
		int CalcUndevelopedPieces(const Position &p);

		template<EvalColour C>
		int CalcUnmovedPawns(const Position &p);
	};
};

#endif