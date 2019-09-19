#pragma once

#include <map>


namespace medusa
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
			parameters[RookValue]		 = 500;
			parameters[PawnValue]		 = 100;
			parameters[BishopValue]		 = 300;
			parameters[KnightValue]		 = 300;
			parameters[QueenValue]		 = 900;

			// Other positional factors in centipawns
			parameters[DoubledPawn]      = 25;
			parameters[KnightOutpost]    = 10;
			parameters[DoubledRooks]     = 10;
			parameters[MinorPositioning] = 10;

			// King safety factors in centipawns
			parameters[KingOnEdge]		 = 4;
			parameters[KingInCentre]	 = 5;
			parameters[KingRingAttacked] = 6;
			parameters[KingRingShield]	 = 2;
			
			// Pawn structure factors in centipawns
			parameters[UnmovedPawn]		 = 8;
			parameters[CentralPawn]		 = 9;
			parameters[DoublePawn]		 = 22;
			parameters[AdvancedPawn]	 = 11;

			// Piece coordination and minor piece potential
			// in centipawns
			parameters[RookSemiOpen]	 = 7;
			parameters[RookOpenBonus]	 = 10;
			parameters[UnmovedPiece]	 = 5;
			parameters[BishopControl]	 = 3;
			parameters[KnightOutpost]	 = 20;
			parameters[KnightOnEdge]	 = 10;
		}

		int get(ParameterID id) const
		{
			return parameters.at(id);
		}

		void set(ParameterID id, int value)
		{
			parameters[id] = value;
		}

	private:
		std::map<ParameterID, int> parameters;
	};

};