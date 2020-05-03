#include <random>

#include "utils/bititer.h"
#include "evaluation.h"

namespace Medusa
{
	namespace Evaluation
	{
		
		/// An attempt at some kind of positional score. Not very good.
		template<EvalColour C>
		int CalcPositionalScore(Position &p)
		{
			/// King constants
			const int __king_1 = 20;
			const int __king_2 = 30;
			const int __king_3 = 40;
			/// Minor pieces
			const int __minor_1 = 5;
			const int __minor_2 = 6;
			const int __minor_3 = 7;
			/// Pawns
			const int __pawn_1 = 15;
			const int __pawn_2 = 18;
			const int __pawn_3 = 20;
			/// Rooks
			const int __rook_1 = 45;
			const int __rook_2 = 90;
			/// Our pieces
			auto king = p.PieceBoard(C, KING);
			auto pawns = p.PieceBoard(C, PAWN);
			auto knights = p.PieceBoard(C, KNIGHT);
			auto bishops = p.PieceBoard(C, BISHOP);
			auto rooks = p.PieceBoard(C, ROOK);
			/// Colours
			auto us = Get<C>();
			auto them = ~us;
			/// Their pieces
			auto theirpawns = p.PieceBoard(them, PAWN);
			auto theirknights = p.PieceBoard(them, KNIGHT);
			/// Occupants
			auto occupancy = p.Occupants();
			auto occus = p.Occupants(us);
			auto occthem = p.Occupants(them);
			/// score
			int score = 0;
			/// Get the phase of the game.
			/// Simple rule, if less than 8 pieces then endgame, if 6 unmoved pawns and
			/// a single undeveloped piece then opening or if 2 or more undeveloped pieces,
			/// also opening. Otherwise endgame.
			int number_pieces = occupancy.PopCnt();
			auto pawn_rank_idx = C == White ? 1 : 6;
			auto pawn_rank_bb = ranks[pawn_rank_idx];
			auto unmoved = (pawns & pawn_rank_bb).PopCnt();
			auto back_rank_idx = C == White ? 0 : 7;
			auto back_rank_bb = ranks[back_rank_idx];
			auto undeveloped = ((bishops | knights) & back_rank_bb).PopCnt();
			auto developed = (knights | bishops).PopCnt() - undeveloped;
			auto phase = Middlegame;
			if (number_pieces <= 8)
				phase = Endgame;
			if (unmoved > 5 && undeveloped > 0)
				phase = Opening;
			if (undeveloped > 1)
				phase = Opening;

			/// In the opening, the king should be out of the centre
			int king_on_rim = (king & BB_RIM).PopCnt();
			if (phase != Endgame)
			{
				int king_in_centre = (king & BB_CTR).PopCnt();

				// King in the centre penalty during the opening.
				score -= __king_1 * king_in_centre;
				score += __king_1 * king_on_rim;

				// Squares around the king are attacked? 
				auto kingsqr = BbSqr(king);
				auto around = neighbours[kingsqr];
				auto them = ~Get<C>();
				for (auto it = around.begin(); it != around.end(); it.operator++())
				{
					Square sqr = Square(*it);
					auto bb = SqrBb(sqr);
					score -= __king_1 * p.IsSquareAttacked(sqr, them);
					score += __king_2 * (bb & pawns).PopCnt();
				}
			}
			if (phase == Endgame)
				score -= __king_3 * king_on_rim;

			score -= __minor_1 * undeveloped;

			// A knight on the rim is dim.
			Bitboard knights_on_rim = knights & BB_RIM;
			score -= __minor_1 * knights_on_rim.PopCnt();

			// Bishops on long diagonals bonus (TODO: if they can be
			// kicked or blocked?)
			for (auto it = bishops.begin(); it != bishops.end(); it.operator++())
			{
				auto sqr = Square(*it);
				auto bishop_influence = DirectionAttacks(occupancy, sqr, bishop_directions);
				score += __minor_2 * (bishop_influence & (~BB_CTR_SQR)).PopCnt();
				score += __minor_2 * (bishop_influence & (BB_CTR_SQR)).PopCnt();
			}

			// Knight on advanced outpost unable to be moved since
			// no pawns in the neighbouring files can move towards.
			// Opposite knight not currently attacking the square.
			auto knightsctr = knights & BB_CTR_SQR;
			for (auto it = knightsctr.begin(); it != knightsctr.end(); it.operator++())
			{
				auto sqridx = *it;
				auto sqr = Square(sqridx);
				auto fidx = sqridx % 8;
				auto ridx = sqridx / 8;
				bool no_pawns = true;
				Bitboard ranksahead = 0;
				int dir = (C == White) ? 1 : -1;
				for (int ridx2 = ridx + dir; ridx2 > 0 && ridx2 < 7; ridx2 += dir)
					ranksahead = ranksahead | ranks[ridx2];
				theirpawns &= ranksahead;
				if (fidx > 0)
					no_pawns &= !(files[fidx - 1] & theirpawns);
				if (fidx < 7)
					no_pawns &= !(files[fidx + 1] & theirpawns);

				// No knight attacking the square and no pawns can kick.
				if (!(knight_attacks[sqr] & theirknights) && no_pawns)
					score += __minor_3;
			}

			if (phase != Endgame)
			{
				// pawn structure: unmoved pawns
				score -= __pawn_1 * unmoved;

				// control the centre
				score += __pawn_1 * (BB_CTR_SQR & pawns).PopCnt();
			}

			for (auto it = pawns.begin(); it != pawns.end(); it.operator++())
			{
				auto sqr = Square(*it);
				auto pwnbb = SqrBb(sqr);
				auto file = sqr % 8;
				auto rank = sqr / 8;
				auto filebb = files[file];

				// pawn structure: doubled pawns
				score -= __pawn_2 * (pawns & filebb & (~pwnbb)).PopCnt();

				// Advanced pawns
				if (phase == Endgame)
				{
					// Rank goes from 0 to 7, make it symmetric
					int m = (C == White) ? 1 : -1;
					score += m * (rank - 3) * __pawn_3;
				}
			}

			/// Rook coordination
			auto usnorooks = occus & ~(rooks);

			/// Rook on open files / doubled
			for (auto it = rooks.begin(); it != rooks.end(); it.operator++())
			{
				auto sqr = *it;
				auto rbb = SqrBb(Square(sqr));
				auto fidx = sqr % 8;
				auto fbb = files[fidx];
				if (!(usnorooks & occus & fbb))
				{
					score += __rook_1;
					if (!(occthem & fbb))
						score += __rook_2;
				}
			}

			return score;
		}

		template<EvalColour C>
		int CalcMaterialScore(Position &p)
		{
			int S = 0;

			S += PawnValue * p.PieceBoard(C, PAWN).PopCnt();
			S += KnightValue * p.PieceBoard(C, KNIGHT).PopCnt();
			S += RookValue * p.PieceBoard(C, ROOK).PopCnt();
			S += BishopValue * p.PieceBoard(C, BISHOP).PopCnt();
			S += QueenValue * p.PieceBoard(C, QUEEN).PopCnt();

			return S;
		}

		int StaticScore(Position &p)
		{
			int s_score = CalcMaterialScore<White>(p) - CalcMaterialScore<Black>(p);
			int p_score = CalcPositionalScore<White>(p) - CalcPositionalScore<Black>(p);
			int noise = (rand() % 10) - 10;
			return s_score + p_score + noise;
		}
	}
};