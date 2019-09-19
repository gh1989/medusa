#include <random>

#include "parameters.h"
#include "utils/bititer.h"
#include "evaluation.h"

namespace medusa
{
	namespace eval
	{
		template<EvalColour C>
		int material_score(Position &p, const Parameters &params)
		{
			int S = 0;

			S += params.get(PawnValue) * p.piecebb(C, PAWN).popcnt();
			S += params.get(KnightValue) * p.piecebb(C, KNIGHT).popcnt();
			S += params.get(RookValue) * p.piecebb(C, ROOK).popcnt();
			S += params.get(BishopValue) * p.piecebb(C, BISHOP).popcnt();
			S += params.get(QueenValue) * p.piecebb(C, QUEEN).popcnt();

			return S;
		}

		template<EvalColour C>
		int pawn_structure(Position &p, GamePhase phase, const Parameters &params)
		{
			int score = 0;
			auto pawns = p.piecebb(C, PAWN);

			// Push pawns and get good pawn structure.
			if (phase != Endgame)
			{
				// pawn structure: unmoved pawns
				score -= params.get(UnmovedPawn) * unmovedpawns<C>(p);

				// control the centre
				score += params.get(CentralPawn) * (BB_CTR_SQR & pawns).popcnt();
			}

			for (auto it = pawns.begin(); it != pawns.end(); it.operator++())
			{
				auto sqr = Square(*it);
				auto pwnbb = sqrbb(sqr);
				auto file = sqr % 8;
				auto rank = sqr / 8;
				auto filebb = files[file];

				// pawn structure: doubled pawns
				score -= params.get(DoubledPawn) * (pawns & filebb & (~pwnbb)).popcnt();

				// Advanced pawns
				if (phase == Endgame)
				{
					// Rank goes from 0 to 7, make it symmetric
					int m = (C == White) ? 1 : -1;
					score += m * (rank - 3) * params.get(AdvancedPawn);
				}
			}

			return score;
		}

		template<EvalColour C>
		int king_position(Position &p, GamePhase phase, const Parameters &params)
		{
			int score = 0;
			auto king = p.piecebb(C, KING);
			int king_in_centre = (king & BB_CTR).popcnt();
			int king_on_rim    = (king & BB_RIM).popcnt();

			// 1. In the opening, the king should be out of the centre
			if (phase != Endgame)
			{
				// King in the centre penalty during the opening.
				score -= params.get(KingInCentre) * king_in_centre;
				score += params.get(KingOnEdge) * king_on_rim;

				// Squares around the king are attacked? 
				auto kingsqr = bbsqr(king);
				auto around = neighbours[kingsqr];
				auto them = ~get<C>();
				auto pawns = p.piecebb(C, PAWN);
				for (auto it = around.begin(); it != around.end(); it.operator++())
				{
					Square sqr = Square(*it);
					auto bb = sqrbb(sqr);
					score -= params.get(KingRingAttacked) * p.is_square_attacked(sqr, them);
					score += params.get(KingRingShield) * (bb & pawns).popcnt();
				}
			}

			// 2. In the endgame calculate nothing, apart from trapped pieces
			// or knight on the rim in some cases.
			if (phase == Endgame)
			{
				score -= params.get(KingOnEdge) * king_on_rim;
			}

			return score;
		}
			
		template<EvalColour C>
		int minor_pieces(Position &p, GamePhase phase, const Parameters &params)
		{
			int score = 0;
			auto knights = p.piecebb(C, KNIGHT);

			// A knight on the rim is dim.
			Bitboard knights_on_rim = knights & BB_RIM;
			score -= params.get(KnightOnEdge) * knights_on_rim.popcnt();
			auto undeveloped = undevelopedpcs<C>(p);

			// 1. In the opening, develop the minor pieces to good squares.
			if (phase == Opening)
			{	
				// Pieces that are not moved yet.
				score -= params.get(UnmovedPiece) * undeveloped;
			}

			// 2. In the middlegame, nothing changes, knight might want a more
			// advanced outpost.
			if (phase != Endgame)
			{
				score -= params.get(UnmovedPiece) * undeveloped;
				auto us = get<C>();
				auto them = ~us;
				auto occupancy = p.occupants();

				// Bishops on long diagonals bonus (TODO: if they can be
				// kicked or blocked?)
				auto bishops = p.piecebb(C, BISHOP);
				for (auto it = bishops.begin(); it != bishops.end(); it.operator++())
				{
					auto sqr = Square(*it);
					auto bishop_influence = direction_attacks(occupancy, sqr, bishop_directions);
					score += params.get(BishopControl) * (bishop_influence & (~BB_CTR_SQR)).popcnt();
					score += params.get(BishopControl) * (bishop_influence & (BB_CTR_SQR)).popcnt();
				}

				// Knight on advanced outpost unable to be moved since
				// no pawns in the neighbouring files can move towards.
				// Opposite knight not currently attacking the square.
				auto knightsctr = p.piecebb(C, KNIGHT) & BB_CTR_SQR;
				auto theirpawns = p.piecebb(them, PAWN);
				auto theirknights = p.piecebb(them, KNIGHT);
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
						score += params.get(KnightOutpost);
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
		int developedpcs(const Position &p)
		{
			auto knights = p.piecebb(C, Piece::KNIGHT);
			auto bishops = p.piecebb(C, Piece::BISHOP);
			return (knights | bishops).popcnt() - undevelopedpcs<C>();
		}

		template<EvalColour C>
		int undevelopedpcs(const Position &p)
		{
			auto back_rank_idx = C == White ? 0 : 7;
			auto back_rank_bb = ranks[back_rank_idx];
			auto knights = p.piecebb(C, Piece::KNIGHT);
			auto bishops = p.piecebb(C, Piece::BISHOP);
			return ((bishops | knights) & back_rank_bb).popcnt();
		}

		template<EvalColour C>
		int unmovedpawns(const Position &p)
		{
			auto pawn_rank_idx = C == White ? 1 : 6;
			auto pawn_rank_bb = ranks[pawn_rank_idx];
			return  (p.piecebb(C, Piece::PAWN) & pawn_rank_bb).popcnt();
		}

		template<EvalColour C>
		GamePhase game_phase(const Position &p)
		{
			auto occupancy = p.occupants(C);
			int number_pieces = occupancy.popcnt();
			int unmoved = unmovedpawns<C>(p);
			int undeveloped = undevelopedpcs<C>(p);

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
		int coordination(Position &p, GamePhase phase, const Parameters &params)
		{
			int score = 0;
			auto us = get<C>();
			auto them = ~us;
			auto occus = p.occupants(us);
			auto occthem = p.occupants(them);
			auto rooks = p.piecebb(C, ROOK);
			if (phase != Endgame)
			{
				auto usnorooks = occus & ~(rooks);
				// Rook on open files / doubled
				for (auto it = rooks.begin(); it != rooks.end(); it.operator++())
				{
					auto sqr = *it;
					auto rbb = sqrbb(Square(sqr));
					auto fidx = sqr % 8;
					auto fbb = files[fidx];
					if (!(usnorooks & occus & fbb))
					{
						score += params.get(RookSemiOpen);
						if (!(occthem & fbb))
							score += params.get(RookOpenBonus);
					}
				}
			}

			return score;
		}

		template<EvalColour C>
		int space(Position &p, GamePhase phase, const Parameters &params)
		{
			// TODO: ...
			return 0;
		}

		template<EvalColour C>
		int static_score(Position &p, const Parameters &params)
		{
			GamePhase phase = game_phase<C>(p);
			int S = 0;

			// 1. Material
			int s_material = material_score<C>(p, params);
      		S += s_material;

			// 2. Pawn structure
			int s_pawn_structure = pawn_structure<C>(p, phase, params);
			S += s_pawn_structure;
			
			// 3. King position
			int s_king_position = king_position<C>(p, phase, params);
			S += s_king_position;

			// 4. Minor piece long term potential
			int s_minor_pieces = minor_pieces<C>(p, phase, params);
			S += s_minor_pieces;

			// 5. Co-ordination
			int s_coordination = coordination<C>(p, phase, params);
			S += s_coordination;

			// 6. Space
			int s_space = space<C>(p, phase, params);
			S += s_space;
/*
#ifdef _DEBUG
			std::cout << "Minor piece: " << s_minor_pieces << std::endl;
			std::cout << "King position: " << s_king_position << std::endl;
			std::cout << "Pawn structure: " << s_pawn_structure << std::endl;
			std::cout << "Co-ordination: " << s_coordination << std::endl;
			std::cout << "Material: " << s_material << std::endl;
			std::cout << "Space: " << s_space << std::endl;
#endif
*/
			return S;
		}

		int static_score(Position &p, const Parameters &params)
		{
			int s_white = static_score<White>(p, params);
			int s_black = static_score<Black>(p, params);
/*
#ifdef _DEBUG
			std::cout << "White: " << s_white << std::endl;
			std::cout << "Black: " << s_black << std::endl;
			std::cout << "Score: " << s_white-s_black << std::endl;
#endif
*/
			return  s_white - s_black;
		}
	}
};