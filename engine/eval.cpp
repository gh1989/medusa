#include <random>
#include "eval.h"

namespace Eval
{
	typedef Bitboard::Square Sq;

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
		S += PAWN_SCORE * p.get_piece_bitboard	(C, Piece::PAWN).on_bits();
		S += KNIGHT_SCORE * p.get_piece_bitboard(C, Piece::KNIGHT).on_bits();
		S += ROOK_SCORE * p.get_piece_bitboard	(C, Piece::ROOK).on_bits();
		S += BISHOP_SCORE * p.get_piece_bitboard(C, Piece::BISHOP).on_bits();
		S += QUEEN_SCORE * p.get_piece_bitboard	(C, Piece::QUEEN).on_bits();

		return S;
	}

	Bitboard king_side_home(EvalColour c)
	{
		if (c == White)
			return Bitboard::get_sq(Sq::g1, Sq::h1);
		return Bitboard::get_sq(Sq::g8, Sq::h8);
	}

	Bitboard queen_side_home(EvalColour c)
	{
		if (c == White)
			return Bitboard::get_sq(Sq::b1, Sq::c1);
		return Bitboard::get_sq(Sq::b8, Sq::c8);
	}

	Bitboard king_side_shield(EvalColour c)
	{
		if(c == White)
			return Bitboard::get_sq(Sq::f2, Sq::g2, Sq::h2);
		return Bitboard::get_sq(Sq::f7, Sq::g7, Sq::h7);
	}
	
	Bitboard queen_side_shield(EvalColour c)
	{
		if (c == White)
			return Bitboard::get_sq(Sq::a2, Sq::b2, Sq::c2);
		return Bitboard::get_sq(Sq::a7, Sq::b7, Sq::c7);
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

		auto king = p.get_piece_bitboard(C, Piece::KING);
		auto pawns = p.get_piece_bitboard(C, Piece::PAWN);
		if ((king & king_side_home(C)).on_bits())
			S += K * (ks_shield & pawns).on_bits();
		if ((king & queen_side_home(C)).on_bits())
			S += K * (qs_shield & pawns).on_bits();

		if (phase != Endgame)
			return S;

		// Get the king in the centre in the end game.
		const auto centre = Bitboard(0x3c24243c0000);
		S += (king & centre).on_bits() * 100;

		return S;
	}

	Bitboard pawn_squares(EvalColour c)
	{
		if (c == White)
			return Bitboard::get_sq(Sq::e4, Sq::d4);
		return Bitboard::get_sq(Sq::d5, Sq::e5);
	}

	template<EvalColour C>
	int pawn_score(Position &p, GamePhase phase)
	{
		int S = 0;
		auto pawns = p.get_piece_bitboard(C, Piece::PAWN);

		// Doubled pawns
		auto files = Bitboard::get_files();
		for (int file = Bitboard::A_FILE; file < Bitboard::NUMBER_FILES; file++)
		{
			int K = 22;
			auto file_bb = files.data[file];
			int pawns_on_file = (file_bb & pawns).on_bits();
			S -= std::max(pawns_on_file - 1, 0)*K;
		}

		// Good opening/middlegame squares
		if (phase != Endgame)
		{
			int K = 16;
			S += K * (pawn_squares(C) & pawns).on_bits();
		}

		return S;
	}

	Bitboard outposts(EvalColour c)
	{
		if (c == White)
			return Bitboard::get_sq(Sq::e6, Sq::e5, Sq::d6, Sq::d5, Sq::c5, Sq::f5, Sq::c6, Sq::f6);
		return Bitboard::get_sq(Sq::e4, Sq::e4, Sq::d4, Sq::d3, Sq::c4, Sq::f4, Sq::c3, Sq::f3);
	}

	template<EvalColour C>
	int knight_score(Position &p, GamePhase phase)
	{
		int S = 0;
		int K = 85;
		auto knights = p.get_piece_bitboard(C, Piece::KNIGHT);
		auto outpost_bb = outposts(C);
		S += K * (outpost_bb & knights).on_bits();
		return S;
	}

	template<EvalColour C>
	int bishop_score(Position &p, GamePhase phase)
	{
		int S = 0;
		int K = 45;
		auto bishops = p.get_piece_bitboard(C, Piece::BISHOP);
		auto outpost_bb = outposts(C);
		S += K * (outpost_bb & bishops).on_bits();
		return S;
	}
	
	template<EvalColour C>
	int rook_score(Position &p, GamePhase phase)
	{
		int S = 0;
		int K = 50;
		auto rooks = p.get_piece_bitboard(C, Piece::ROOK);
		auto occup = p.occupants(C);

		// Rooks on open files, big score for doubled on open files.
		auto files = Bitboard::get_files();
		for (int file = Bitboard::A_FILE; file < Bitboard::NUMBER_FILES; file++)
		{
			auto file_bb = files.data[file];
			int rooks_on_file = (rooks & file_bb).on_bits();
			if (!rooks_on_file)
				continue;
			bool open_file = (file_bb & ( occup & (~rooks))).on_bits() == 0;
			if (open_file)
				S += rooks_on_file * rooks_on_file * K;
		}
			   
		return S;
	}

	template<EvalColour C>
	GamePhase game_phase(const Position &p)
	{
		auto occupancy = p.occupants(C);
		int number_pieces = occupancy.on_bits();
		if (number_pieces <= 8)
			return GamePhase::Endgame;
			   
		auto ranks = Bitboard::get_ranks();
		auto pawn_rank = C == White ? Bitboard::RANK_2 : Bitboard::RANK_7;
		auto pawn_rank_bb = ranks.data[pawn_rank];
		int unmoved_pawns = (p.get_piece_bitboard(C, Piece::PAWN) & pawn_rank_bb).on_bits();
		if (unmoved_pawns > 5)
			return GamePhase::Opening;
		
		auto back_rank = C == White ? Bitboard::RANK_1 : Bitboard::RANK_8;
		auto knights = p.get_piece_bitboard(C, Piece::KNIGHT);
		auto bishops = p.get_piece_bitboard(C, Piece::BISHOP);
		int undeveloped_pc = ((bishops | knights) & back_rank).on_bits();
		if (undeveloped_pc > 1)
			return GamePhase::Opening;

		return GamePhase::Middlegame;
	}

	template<EvalColour C>
	int static_score(Position &p)
	{
		GamePhase phase = game_phase<C>(p);

		// Material score
		int S = 0;

		S += material_score<C>(p);
		S += king_score    <C>(p, phase);
		S += pawn_score    <C>(p, phase);
		S += rook_score    <C>(p, phase);
		S += knight_score  <C>(p, phase);
		S += bishop_score  <C>(p, phase);

		return S;
	}

	int static_score(Position &p)
	{
		return static_score<White>(p) - static_score<Black>(p);
	}
};