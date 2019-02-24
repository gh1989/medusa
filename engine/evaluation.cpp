#include <random>
#include "evaluation.h"

namespace medusa
{
	namespace eval
	{
		enum GamePhase
		{
			Opening,
			Middlegame,
			Endgame
		};

		enum EvalColour
		{
			White,
			Black
		};

		template<EvalColour C>
		int material_score(Position &p)
		{
			int S = 0;

			// Material
			S += PAWN_SCORE * p.piecebb(C, Piece::PAWN).popcnt();
			S += KNIGHT_SCORE * p.piecebb(C, Piece::KNIGHT).popcnt();
			S += ROOK_SCORE * p.piecebb(C, Piece::ROOK).popcnt();
			S += BISHOP_SCORE * p.piecebb(C, Piece::BISHOP).popcnt();
			S += QUEEN_SCORE * p.piecebb(C, Piece::QUEEN).popcnt();

			return S;
		}

		Bitboard king_side_home(EvalColour c)
		{
			if (c == White)
				return medusa::get_sq(g1, h1);
			return get_sq(g8, h8);
		}

		Bitboard queen_side_home(EvalColour c)
		{
			if (c == White)
				return medusa::get_sq(b1, c1);
			return medusa::get_sq(b8, c8);
		}

		Bitboard king_side_shield(EvalColour c)
		{
			if (c == White)
				return medusa::get_sq(f2, g2, h2);
			return medusa::get_sq(f7, g7, h7);
		}

		Bitboard queen_side_shield(EvalColour c)
		{
			if (c == White)
				return medusa::get_sq(a2, b2, c2);
			return medusa::get_sq(a7, b7, c7);
		}

		template<EvalColour C>
		int king_score(Position &p, GamePhase phase)
		{
			int S = 0;
			int K = phase == Endgame ? 0 : 15;

			// King over in the corner covered by pawns gives a small safety bonus to
			// the score.
			auto ks_shield = king_side_shield(C);
			auto qs_shield = queen_side_shield(C);

			auto king = p.piecebb(C, Piece::KING);
			auto pawns = p.piecebb(C, Piece::PAWN);
			if ((king & king_side_home(C)).popcnt())
				S += K * (ks_shield & pawns).popcnt();
			if ((king & queen_side_home(C)).popcnt())
				S += K * (qs_shield & pawns).popcnt();

			if (phase != Endgame)
				return S;

			// Get the king in the centre in the end game.
			const auto centre = Bitboard(0x3c24243c0000);
			S += (king & centre).popcnt() * 100;

			return S;
		}

		Bitboard pawn_squares(EvalColour c)
		{
			if (c == White)
				return medusa::get_sq(e4, d4);
			return medusa::get_sq(d5, e5);
		}

		template<EvalColour C>
		int pawn_score(Position &p, GamePhase phase)
		{
			int S = 0;
			auto pawns = p.piecebb(C, Piece::PAWN);

			// Doubled pawns
			for (int file = 0; file < 8; file++)
			{
				int K = 22;
				auto file_bb = files[file];
				int pawns_on_file = (file_bb & pawns).popcnt();
				S -= std::max(pawns_on_file - 1, 0)*K;
			}

			// Good opening/middlegame squares
			if (phase != Endgame)
			{
				int K = 16;
				S += K * (pawn_squares(C) & pawns).popcnt();
			}

			return S;
		}

		Bitboard outposts(EvalColour c)
		{
			if (c == White)
				return medusa::get_sq(e6, e5, d6, d5, c5, f5, c6, f6);
			return medusa::get_sq(e4, e4, d4, d3, c4, f4, c3, f3);
		}

		template<EvalColour C>
		int knight_score(Position &p, GamePhase phase)
		{
			int S = 0;
			int K = 0;
			auto knights = p.piecebb(C, Piece::KNIGHT);
			auto outpost_bb = outposts(C);
			S += K * (outpost_bb & knights).popcnt();
			return S;
		}

		template<EvalColour C>
		int bishop_score(Position &p, GamePhase phase)
		{
			int S = 0;
			int K = 45;
			auto bishops = p.piecebb(C, Piece::BISHOP);
			auto outpost_bb = outposts(C);
			S += K * (outpost_bb & bishops).popcnt();
			return S;
		}

		template<EvalColour C>
		int rook_score(Position &p, GamePhase phase)
		{
			int S = 0;
			int K = 50;
			auto rooks = p.piecebb(C, Piece::ROOK);
			auto occup = p.occupants(C);

			// Rooks on open files, big score for doubled on open files.
			for (int file = 0; file < 8; file++)
			{
				auto file_bb = files[file];
				int rooks_on_file = (rooks & file_bb).popcnt();
				if (!rooks_on_file)
					continue;
				bool open_file = (file_bb & (occup & (~rooks))).popcnt() == 0;
				if (open_file)
					S += rooks_on_file * rooks_on_file * K;
			}

			return S;
		}

		template<EvalColour C>
		GamePhase game_phase(const Position &p)
		{
			auto occupancy = p.occupants(C);
			int number_pieces = occupancy.popcnt();
			if (number_pieces <= 8)
				return GamePhase::Endgame;

			auto pawn_rank_idx = C == White ? 1 : 6;
			auto pawn_rank_bb = ranks[pawn_rank_idx];
			int unmoved_pawns = (p.piecebb(C, Piece::PAWN) & pawn_rank_bb).popcnt();
			if (unmoved_pawns > 5)
				return GamePhase::Opening;

			auto back_rank_idx = C == White ? 0 : 7;
			auto back_rank_bb = ranks[back_rank_idx];
			auto knights = p.piecebb(C, Piece::KNIGHT);
			auto bishops = p.piecebb(C, Piece::BISHOP);
			int undeveloped_pc = ((bishops | knights) & back_rank_bb).popcnt();
			if (undeveloped_pc > 1)
				return GamePhase::Opening;

			return GamePhase::Middlegame;
		}

		template<EvalColour C>
		int pawn_structure(Position &p, GamePhase phase)
		{
			int S = 0;
			auto pawns = p.piecebb(C, Piece::PAWN);

			// Doubled pawns
			for (int file = 0; file < 8; file++)
			{
				int K = 50; // Half a pawn penalty for a doubled pawn.
				auto file_bb = files[file];
				int pawns_on_file = (file_bb & pawns).popcnt();
				S -= std::max(pawns_on_file - 1, 0)*K;
			}

			return S;
		}

		template<EvalColour C>
		int king_safety(Position &p, GamePhase phase)
		{
			return king_score<C>(p, phase);
		}

		template<EvalColour C>
		int minor_pieces(Position &p, GamePhase phase)
		{
			return bishop_score<C>(p, phase) + knight_score<C>(p, phase);
		}

		template<EvalColour C>
		int coordination(Position &p, GamePhase phase)
		{
			return rook_score<C>(p, phase);
		}

		template<EvalColour C>
		int space(Position &p, GamePhase phase)
		{
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
			// 3. King safety
			S += king_safety<C>(p, phase);
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