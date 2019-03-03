#include <random>

#include "utils/bititer.h"
#include "evaluation.h"

namespace medusa
{
	namespace eval
	{
		
		template<EvalColour C>
		int material_score(Position &p)
		{
			int S = 0;

			S += PAWN_SCORE * p.piecebb(C, PAWN).popcnt();
			S += KNIGHT_SCORE * p.piecebb(C, KNIGHT).popcnt();
			S += ROOK_SCORE * p.piecebb(C, ROOK).popcnt();
			S += BISHOP_SCORE * p.piecebb(C, BISHOP).popcnt();
			S += QUEEN_SCORE * p.piecebb(C, QUEEN).popcnt();

			return S;
		}

		template<EvalColour C>
		int pawn_structure(Position &p, GamePhase phase)
		{
			int score = 0;

			// Push pawns and get good pawn structure.
			if (phase != Endgame)
			{
				// pawn structure: unmoved pawns
				score -= 10 * unmovedpawns<C>(p);
			}

			auto pawns = p.piecebb(C, PAWN);
			for (auto it = pawns.begin(); it != pawns.end(); it.operator++())
			{
				auto sqr = Square(*it);
				auto pwnbb = sqrbb(sqr);
				auto file = sqr % 8;
				auto filebb = files[file];

				// pawn structure: doubled pawns
				score -= 10 * (pawns & filebb & (~pwnbb)).popcnt();
			}

			return score;
		}

		template<EvalColour C>
		int king_position(Position &p, GamePhase phase)
		{
			int score = 0;
			auto king = p.piecebb(C, KING);
			int king_in_centre = (king & BB_CTR).popcnt();
			int king_on_rim    = (king & BB_RIM).popcnt();

			// 1. In the opening, the king should be out of the centre
			if (phase != Endgame)
			{
				// King in the centre penalty during the opening.
				score -= 10 * king_in_centre;
				
				// Squares around the king are attacked? 
				auto kingsqr = bbsqr(king);
				auto around = neighbours[kingsqr];
				for (auto it = around.begin(); it != around.end(); it.operator++())
				{
					Square sqr = Square(*it);
					score -= 10 * p.is_square_attacked(sqr, ~get<C>());
				}
			}

			// 2. In the endgame calculate nothing, apart from trapped pieces
			// or knight on the rim in some cases.
			if (phase == Endgame)
			{
				score -= 10 * king_on_rim;
			}

			return score;
		}
			
		template<EvalColour C>
		int minor_pieces(Position &p, GamePhase phase)
		{
			int score = 0;
			auto knights = p.piecebb(C, KNIGHT);

			// A knight on the rim is dim.
			Bitboard knights_on_rim = knights & BB_RIM;
			score -= 10 * knights_on_rim.popcnt();
			auto undeveloped = undevelopedpcs<C>(p);

			// 1. In the opening, develop the minor pieces to good squares.
			if (phase == Opening)
			{	
				// Pieces that are not moved yet.
				score -= 10 * undeveloped;
			}

			// 2. In the middlegame, nothing changes, knight might want a more
			// advanced outpost.
			if (phase == Middlegame)
			{
				score -= 10 * undeveloped;
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
		int coordination(Position &p, GamePhase phase)
		{
			// TODO: ...
			return 0;
		}

		template<EvalColour C>
		int space(Position &p, GamePhase phase)
		{
			// TODO: ...
			return 0;
		}

		template<EvalColour C>
		int static_score(Position &p)
		{
			GamePhase phase = game_phase<C>(p);
			int S = 0;

			// 1. Material
			S += material_score<C>(p);

			// 2. Pawn structure
			S += pawn_structure<C>(p, phase);

			// 3. King position
			S += king_position<C>(p, phase);

			// 4. Minor piece long term potential
			S += minor_pieces<C>(p, phase);

			// 5. Co-ordination
			S += coordination<C>(p, phase);

			// 6. Space
			S += space<C>(p, phase);

			return S;
		}

		int static_score(Position &p)
		{
			return static_score<White>(p) - static_score<Black>(p);
		}
	}
};