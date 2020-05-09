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
			/// constants
			const int _ckctr = 20;
			const int _ckrim = 30;
			const int _ckatt = 30;
			const int _ckdef = 30;
			const int _cnrim = 5;
			const int _cnund = 6;
			const int _cbinf = 3;
			const int _cnctl = 7;
			const int _cpctl = 15;
			const int _cpund = 18;
			const int _cpdbl = 20;
			const int _cpadv = 45;
			const int _cropn = 90;
			const int _crdbl = 90;

			/// Our pieces
			auto kng = p.PieceBoard(C, KING);
			auto pwn = p.PieceBoard(C, PAWN);
			auto knt = p.PieceBoard(C, KNIGHT);
			auto bsh = p.PieceBoard(C, BISHOP);
			auto rks = p.PieceBoard(C, ROOK);
			/// Colours
			auto ours = Get<C>();
			auto them = ~ours;
			/// Their pieces
			auto theirpawns = p.PieceBoard(them, PAWN);
			auto theirknights = p.PieceBoard(them, KNIGHT);
			/// Occupants
			auto occupancy	= p.Occupants();
			auto occus		= p.Occupants(ours);
			auto occthem	= p.Occupants(them);
			/// score
			int score = 0;

			/// Get the phase of the game.
			/// Simple rule, if less than 8 pieces then endgame, if 6 unmoved pawns and
			/// a single undeveloped piece then opening or if 2 or more undeveloped pieces,
			/// also opening. Otherwise endgame.
			int number_pieces		= occupancy.PopCnt();
			auto pawn_rank_idx		= C == White ? 1 : 6;
			auto pawn_rank_bb		= ranks[pawn_rank_idx];
			auto unmoved			= (pwn & pawn_rank_bb).PopCnt();
			auto back_rank_idx		= C == White ? 0 : 7;
			auto back_rank_bb		= ranks[back_rank_idx];
			auto undeveloped		= ((bsh | knt) & back_rank_bb).PopCnt();
			auto developed			= (bsh | knt).PopCnt() - undeveloped;

			auto phase = Middlegame;
			if (number_pieces <= 8)
				phase = Endgame;
			if (unmoved > 5 && undeveloped > 0)
				phase = Opening;
			if (undeveloped > 1)
				phase = Opening;

			/// In the opening, the king should be out of the centre
			int nkrim = (kng & BB_RIM).PopCnt();
			if (phase != Endgame)
			{
				int nkctr = (kng & BB_CTR).PopCnt();
				score -= _ckctr * nkctr;
				score += _ckrim * nkrim;
				// Squares around the king are attacked? 
				auto ksqr = BbSqr(kng);
				auto knbr = neighbours[ksqr];
				for (auto it = knbr.begin(); it != knbr.end(); it.operator++())
				{
					auto _sqr = Square(*it);
					auto _bb = SqrBb(_sqr);
					score -= _ckatt * p.IsSquareAttacked(_sqr, them);
					score += _ckdef * (_bb & pwn).PopCnt();
				}
			}
			if (phase == Endgame)
				score -= _ckrim * nkrim;

			score -= _cnund * undeveloped;

			// A knight on the rim is dim.
			Bitboard _nrim = knt & BB_RIM;
			score -= _cnrim * _nrim.PopCnt();

			// Bishops on long diagonals bonus (TODO: if they can be
			// kicked or blocked?)
			for (auto it = bsh.begin(); it != bsh.end(); it.operator++())
			{
				auto sqr = Square(*it);
				auto _binf = DirectionAttacks(occupancy, sqr, bishop_directions);
				score += _cbinf * (_binf & (~BB_CTR_SQR)).PopCnt();
				score += _cbinf * (_binf & (BB_CTR_SQR)).PopCnt();
			}

			// Knight on advanced outpost unable to be moved since
			// no pawns in the neighbouring files can move towards.
			// Opposite knight not currently attacking the square.
			auto kntctr = knt & BB_CTR_SQR;
			for (auto it = kntctr.begin(); it != kntctr.end(); it.operator++())
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
					score += _cnctl;
			}

			if (phase != Endgame)
			{
				// pawn structure: unmoved pawns
				score -= _cpund * unmoved;

				// control the centre
				score += _cpctl * (BB_CTR_SQR & pwn).PopCnt();
			}

			for (auto it = pwn.begin(); it != pwn.end(); it.operator++())
			{
				auto sqr = Square(*it);
				auto pwnbb = SqrBb(sqr);
				auto file = sqr % 8;
				auto rank = sqr / 8;
				auto filebb = files[file];

				// pawn structure: doubled pawns
				score -= _cpdbl * (pwn & filebb & (~pwnbb)).PopCnt();

				// Advanced pawns
				if (phase == Endgame)
				{
					// Rank goes from 0 to 7, make it symmetric
					int m = (C == White) ? 1 : -1;
					score += m * (rank - 3) * _cpadv;
				}
			}

			/// Rook coordination
			auto usnorooks = occus & ~(rks);

			/// Rook on open files / doubled
			for (auto it = rks.begin(); it != rks.end(); it.operator++())
			{
				auto sqr = *it;
				auto rbb = SqrBb(Square(sqr));
				auto fidx = sqr % 8;
				auto fbb = files[fidx];
				if (!(usnorooks & occus & fbb))
				{
					score += _cropn;
					if (!(occthem & fbb))
						score += _crdbl;
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