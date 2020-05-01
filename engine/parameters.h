#pragma once

#include <map>


namespace Medusa
{
	enum ParameterID
	{
		RookValue = 0,
		PawnValue, 
		BishopValue,
		KnightValue,
		QueenValue,
		KingOnEdge,
		KingInCentre,
		KingRingAttacked,
		KingRingShield,
		DoubledPawn,
		KnightOutpost,
		MinorPositioning,
		DoubledRooks,
		UnmovedPawn,
		CentralPawn,
		DoublePawn,
		AdvancedPawn,
		RookSemiOpen,
		RookOpenBonus,
		UnmovedPiece,
		BishopControl,
		KnightOnEdge,
	};

	class Parameters
	{
	public:
		Parameters()
		{
			// Piece material constants in centipawns
			parameters[RookValue]		 = 550;
			parameters[PawnValue]		 = 110;
			parameters[BishopValue]		 = 330;
			parameters[KnightValue]		 = 300;
			parameters[QueenValue]		 = 910;

			// Other positional factors in centipawns
			parameters[DoubledPawn]      = 11;
			parameters[DoubledRooks]     = 13;
			parameters[MinorPositioning] = 3;

			// King safety factors in centipawns
			parameters[KingOnEdge]		 = 10;
			parameters[KingInCentre]	 = 10;
			parameters[KingRingAttacked] = 6;
			parameters[KingRingShield]	 = 2;
			
			// Pawn structure factors in centipawns
			parameters[UnmovedPawn]		 = 8;
			parameters[CentralPawn]		 = 10;
			parameters[DoublePawn]		 = 3;
			parameters[AdvancedPawn]	 = 7;

			// Piece coordination and minor piece potential
			// in centipawns
			parameters[RookSemiOpen]	 = 10;
			parameters[RookOpenBonus]	 = 13;
			parameters[UnmovedPiece]	 = 5;
			parameters[BishopControl]	 = 8;
			parameters[KnightOutpost]	 = 4;
			parameters[KnightOnEdge]	 = 6;
		}

		int Get(ParameterID id) const
		{
			return parameters.at(id);
		}

		void Set(ParameterID id, int value)
		{
			parameters[id] = value;
		}

	private:
		std::map<ParameterID, int> parameters;
	};

};