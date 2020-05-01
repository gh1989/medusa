#include <random>

#include "parameters.h"
#include "utils/bititer.h"
#include "evaluation.h"

namespace Medusa
{
	namespace Evaluation
	{
		template<EvalColour C>
		int CalcMaterialScore(Position &p, const Parameters &params)
		{
			int S = 0;

			S += params.Get(PawnValue) * p.PieceBoard(C, PAWN).PopCnt();
			S += params.Get(KnightValue) * p.PieceBoard(C, KNIGHT).PopCnt();
			S += params.Get(RookValue) * p.PieceBoard(C, ROOK).PopCnt();
			S += params.Get(BishopValue) * p.PieceBoard(C, BISHOP).PopCnt();
			S += params.Get(QueenValue) * p.PieceBoard(C, QUEEN).PopCnt();

			return S;
		}

		template<EvalColour C>
		int CalcPawnStructureScore(Position &p, GamePhase phase, const Parameters &params)
		{
			int score = 0;
			auto pawns = p.PieceBoard(C, PAWN);

			// Push pawns and get good pawn structure.
			if (phase != Endgame)
			{
				// pawn structure: unmoved pawns
				score -= params.Get(UnmovedPawn) * CalcUnmovedPawns<C>(p);

				// control the centre
				score += params.Get(CentralPawn) * (BB_CTR_SQR & pawns).PopCnt();
			}

			for (auto it = pawns.begin(); it != pawns.end(); it.operator++())
			{
				auto sqr = Square(*it);
				auto pwnbb = SqrBb(sqr);
				auto file = sqr % 8;
				auto rank = sqr / 8;
				auto filebb = files[file];

				// pawn structure: doubled pawns
				score -= params.Get(DoubledPawn) * (pawns & filebb & (~pwnbb)).PopCnt();

				// Advanced pawns
				if (phase == Endgame)
				{
					// Rank goes from 0 to 7, make it symmetric
					int m = (C == White) ? 1 : -1;
					score += m * (rank - 3) * params.Get(AdvancedPawn);
				}
			}

			return score;
		}

		template<EvalColour C>
		int CalcKingPositionScore(Position &p, GamePhase phase, const Parameters &params)
		{
			int score = 0;
			auto king = p.PieceBoard(C, KING);
			int king_in_centre = (king & BB_CTR).PopCnt();
			int king_on_rim    = (king & BB_RIM).PopCnt();

			// 1. In the opening, the king should be out of the centre
			if (phase != Endgame)
			{
				// King in the centre penalty during the opening.
				score -= params.Get(KingInCentre) * king_in_centre;
				score += params.Get(KingOnEdge) * king_on_rim;

				// Squares around the king are attacked? 
				auto kingsqr = BbSqr(king);
				auto around = neighbours[kingsqr];
				auto them = ~Get<C>();
				auto pawns = p.PieceBoard(C, PAWN);
				for (auto it = around.begin(); it != around.end(); it.operator++())
				{
					Square sqr = Square(*it);
					auto bb = SqrBb(sqr);
					score -= params.Get(KingRingAttacked) * p.IsSquareAttacked(sqr, them);
					score += params.Get(KingRingShield) * (bb & pawns).PopCnt();
				}
			}

			// 2. In the endgame calculate nothing, apart from trapped pieces
			// or knight on the rim in some cases.
			if (phase == Endgame)
			{
				score -= params.Get(KingOnEdge) * king_on_rim;
			}

			return score;
		}
			
		template<EvalColour C>
		int CalcMinorPiecePotentialScore(Position &p, GamePhase phase, const Parameters &params)
		{
			int score = 0;
			auto knights = p.PieceBoard(C, KNIGHT);

			// A knight on the rim is dim.
			Bitboard knights_on_rim = knights & BB_RIM;
			score -= params.Get(KnightOnEdge) * knights_on_rim.PopCnt();
			auto undeveloped = CalcUndevelopedPieces<C>(p);

			// 1. In the opening, develop the minor pieces to good squares.
			if (phase == Opening)
			{	
				// Pieces that are not moved yet.
				score -= params.Get(UnmovedPiece) * undeveloped;
			}

			// 2. In the middlegame, nothing changes, knight might want a more
			// advanced outpost.
			if (phase != Endgame)
			{
				score -= params.Get(UnmovedPiece) * undeveloped;
				auto us = Get<C>();
				auto them = ~us;
				auto occupancy = p.Occupants();

				// Bishops on long diagonals bonus (TODO: if they can be
				// kicked or blocked?)
				auto bishops = p.PieceBoard(C, BISHOP);
				for (auto it = bishops.begin(); it != bishops.end(); it.operator++())
				{
					auto sqr = Square(*it);
					auto bishop_influence = DirectionAttacks(occupancy, sqr, bishop_directions);
					score += params.Get(BishopControl) * (bishop_influence & (~BB_CTR_SQR)).PopCnt();
					score += params.Get(BishopControl) * (bishop_influence & (BB_CTR_SQR)).PopCnt();
				}

				// Knight on advanced outpost unable to be moved since
				// no pawns in the neighbouring files can move towards.
				// Opposite knight not currently attacking the square.
				auto knightsctr = p.PieceBoard(C, KNIGHT) & BB_CTR_SQR;
				auto theirpawns = p.PieceBoard(them, PAWN);
				auto theirknights = p.PieceBoard(them, KNIGHT);
				for (auto it = knightsctr.begin(); it != knightsctr.end(); it.operator++())
				{
					auto sqridx = *it;
					auto sqr = Square(sqridx);
					auto fidx = sqridx % 8;
					auto ridx = sqridx / 8;
					bool no_pawns = true;
					Bitboard ranksahead = 0;
					int dir = (C == White) ? 1 : -1;
					for (int ridx2 = ridx+dir; ridx2 > 0 && ridx2 < 7; ridx2 += dir)
						ranksahead = ranksahead | ranks[ridx2];
					theirpawns &= ranksahead;
					if (fidx > 0)
						no_pawns &= !(files[fidx - 1] & theirpawns);
					if (fidx < 7)
						no_pawns &= !(files[fidx + 1] & theirpawns);
					
					// No knight attacking the square and no pawns can kick.
					if (!(knight_attacks[sqr] & theirknights) && no_pawns)
						score += params.Get(KnightOutpost);
				}
			}

			// 3. In the endgame calculate nothing, apart from trapped pieces
			// or knight on the rim in some cases.
			if (phase == Endgame)
			{
				// TODO: ...
			}

			return score;
		}
				
		template <EvalColour C>
		int CalcDevelopedPieces(const Position &p)
		{
			auto knights = p.PieceBoard(C, Piece::KNIGHT);
			auto bishops = p.PieceBoard(C, Piece::BISHOP);
			return (knights | bishops).PopCnt() - CalcUndevelopedPieces<C>();
		}

		template<EvalColour C>
		int CalcUndevelopedPieces(const Position &p)
		{
			auto back_rank_idx = C == White ? 0 : 7;
			auto back_rank_bb = ranks[back_rank_idx];
			auto knights = p.PieceBoard(C, Piece::KNIGHT);
			auto bishops = p.PieceBoard(C, Piece::BISHOP);
			return ((bishops | knights) & back_rank_bb).PopCnt();
		}

		template<EvalColour C>
		int CalcUnmovedPawns(const Position &p)
		{
			auto pawn_rank_idx = C == White ? 1 : 6;
			auto pawn_rank_bb = ranks[pawn_rank_idx];
			return  (p.PieceBoard(C, Piece::PAWN) & pawn_rank_bb).PopCnt();
		}

		template<EvalColour C>
		GamePhase CalcGamePhase(const Position &p)
		{
			auto occupancy = p.Occupants(C);
			int number_pieces = occupancy.PopCnt();
			int unmoved = CalcUnmovedPawns<C>(p);
			int undeveloped = CalcUndevelopedPieces<C>(p);

			// Simple rule, if less than 8 pieces then endgame, if 6 unmoved pawns and
			// a single undeveloped piece then opening or if 2 or more undeveloped pieces,
			// also opening. Otherwise endgame.
			if (number_pieces <= 8)
				return GamePhase::Endgame;
			if (unmoved > 5 && undeveloped > 0)
				return GamePhase::Opening;
			if (undeveloped > 1)
				return GamePhase::Opening;
			return GamePhase::Middlegame;
		}

		
		template<EvalColour C>
		int CalcCoordinationScore(Position &p, GamePhase phase, const Parameters &params)
		{
			int score = 0;
			auto us = Get<C>();
			auto them = ~us;
			auto occus = p.Occupants(us);
			auto occthem = p.Occupants(them);
			auto rooks = p.PieceBoard(C, ROOK);
			if (phase != Endgame)
			{
				auto usnorooks = occus & ~(rooks);
				// Rook on open files / doubled
				for (auto it = rooks.begin(); it != rooks.end(); it.operator++())
				{
					auto sqr = *it;
					auto rbb = SqrBb(Square(sqr));
					auto fidx = sqr % 8;
					auto fbb = files[fidx];
					if (!(usnorooks & occus & fbb))
					{
						score += params.Get(RookSemiOpen);
						if (!(occthem & fbb))
							score += params.Get(RookOpenBonus);
					}
				}
			}

			return score;
		}

		template<EvalColour C>
		int CalcSpaceScore(Position &p, GamePhase phase, const Parameters &params)
		{
			// TODO: ...
			return 0;
		}

		template<EvalColour C>
		int static_score(Position &p, const Parameters &params)
		{
			GamePhase phase = CalcGamePhase<C>(p);
			int S = 0;

			// 1. Material
			int s_material = CalcMaterialScore<C>(p, params);
      		S += s_material;

			// 2. Pawn structure
			int s_pawn_structure = CalcPawnStructureScore<C>(p, phase, params);
			S += s_pawn_structure;
			
			// 3. King position
			int s_king_position = CalcKingPositionScore<C>(p, phase, params);
			S += s_king_position;

			// 4. Minor piece long term potential
			int s_minor_pieces = CalcMinorPiecePotentialScore<C>(p, phase, params);
			S += s_minor_pieces;

			// 5. Co-ordination
			int s_coordination = CalcCoordinationScore<C>(p, phase, params);
			S += s_coordination;

			// 6. Space
			int s_space = CalcSpaceScore<C>(p, phase, params);
			S += s_space;

#ifdef _DEBUG_OFF
			std::cout << "Minor piece: " << s_minor_pieces << std::endl;
			std::cout << "King position: " << s_king_position << std::endl;
			std::cout << "Pawn structure: " << s_pawn_structure << std::endl;
			std::cout << "Co-ordination: " << s_coordination << std::endl;
			std::cout << "Material: " << s_material << std::endl;
			std::cout << "Space: " << s_space << std::endl;
#endif

			return S;
		}

		int StaticScore(Position &p, const Parameters &params)
		{
			int s_white = static_score<White>(p, params);
			int s_black = static_score<Black>(p, params);

#ifdef _DEBUG_OFF
			std::cout << "White: " << s_white << std::endl;
			std::cout << "Black: " << s_black << std::endl;
			std::cout << "Score: " << s_white-s_black << std::endl;
#endif
			return  s_white - s_black;
		}
	}
};