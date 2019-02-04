#include <random>
#include "eval.h"

namespace Eval
{
	int material_score(Position &p)
	{
		int score = 0;

		// Material
		score += PAWN_SCORE * p.get_piece_bitboard(Colour::WHITE, Piece::PAWN).on_bits();
		score -= PAWN_SCORE * p.get_piece_bitboard(Colour::BLACK, Piece::PAWN).on_bits();
		score += KNIGHT_SCORE * p.get_piece_bitboard(Colour::WHITE, Piece::KNIGHT).on_bits();
		score -= KNIGHT_SCORE * p.get_piece_bitboard(Colour::BLACK, Piece::KNIGHT).on_bits();
		score += ROOK_SCORE * p.get_piece_bitboard(Colour::WHITE, Piece::ROOK).on_bits();
		score -= ROOK_SCORE * p.get_piece_bitboard(Colour::BLACK, Piece::ROOK).on_bits();
		score += BISHOP_SCORE * p.get_piece_bitboard(Colour::WHITE, Piece::BISHOP).on_bits();
		score -= BISHOP_SCORE * p.get_piece_bitboard(Colour::BLACK, Piece::BISHOP).on_bits();
		score += QUEEN_SCORE * p.get_piece_bitboard(Colour::WHITE, Piece::QUEEN).on_bits();
		score -= QUEEN_SCORE * p.get_piece_bitboard(Colour::BLACK, Piece::QUEEN).on_bits();

		return score;
	}

	int king_score(Position &p)
	{
		int score = 0;
		int king_saftey = 15;

		// Check to see if the king is safe, surrounded by pawns with those pawns protected
		// this should encourage castling.

		// WHITE KING SAFTEY...
		auto sqrs = Bitboard::get_squares();
		auto kingsideshield = sqrs.data[Bitboard::f2] | sqrs.data[Bitboard::g2] | sqrs.data[Bitboard::h2];
		auto queensideshield = sqrs.data[Bitboard::a2] | sqrs.data[Bitboard::b2] | sqrs.data[Bitboard::c2];

		auto king = p.get_piece_bitboard(Colour::WHITE, Piece::KING);
		auto pawns = p.get_piece_bitboard(Colour::WHITE, Piece::PAWN);
		auto squares = king.non_empty_squares();
		auto king_square = squares.back();
		switch (king_square)
		{
		case(Bitboard::g1):
		case(Bitboard::h1):
		{
			score += (pawns & kingsideshield).on_bits() * king_saftey;
			break;
		}
		case(Bitboard::c1):
		case(Bitboard::b1):
		case(Bitboard::a1):
		{
			score += (pawns & queensideshield).on_bits()  * king_saftey;
			break;
		}
		}

		// BLACK KING SAFTEY...
		kingsideshield = sqrs.data[Bitboard::f7] | sqrs.data[Bitboard::g7] | sqrs.data[Bitboard::h7];
		queensideshield = sqrs.data[Bitboard::a7] | sqrs.data[Bitboard::b7] | sqrs.data[Bitboard::c7];

		king = p.get_piece_bitboard(Colour::BLACK, Piece::KING);
		pawns = p.get_piece_bitboard(Colour::BLACK, Piece::PAWN);
		squares = king.non_empty_squares();
		king_square = squares.back();
		switch (king_square)
		{
		case(Bitboard::g8):
		case(Bitboard::h8):
		{
			score -= (pawns & kingsideshield).on_bits() * king_saftey;
			break;
		}
		case(Bitboard::c8):
		case(Bitboard::b8):
		case(Bitboard::a8):
		{
			score -= (pawns & queensideshield).on_bits()  * king_saftey;
			break;
		}
		}

		return score;
	}

	int pawn_score(Position &p)
	{
		int score = 0;
		int centreo_score = 12;
		int centrei_score = 28;
		auto centreo = Bitboard(0x3c24243c0000);
		auto centrei = Bitboard(0x1818000000);
		auto w_pawns = p.get_piece_bitboard(Colour::WHITE, Piece::PAWN);
		auto b_pawns = p.get_piece_bitboard(Colour::BLACK, Piece::PAWN);
		score += ((w_pawns & centreo).on_bits() - (b_pawns & centreo).on_bits()) * centreo_score;
		score += ((w_pawns & centrei).on_bits() - (b_pawns & centrei).on_bits()) * centrei_score;
		return score;
	}

	int knight_score(Position &p)
	{
		int score = 0;
		int centre_score = 55;
		int advanced_score = 77;
		auto centrew = Bitboard(0x3c00000000);
		auto centreb = Bitboard(0x3c000000);
		auto advancedw = Bitboard(0x3c3c0000000000);
		auto advancedb = Bitboard(0x3c3c00);
		auto w_knights = p.get_piece_bitboard(Colour::WHITE, Piece::KNIGHT);
		auto b_knights = p.get_piece_bitboard(Colour::BLACK, Piece::KNIGHT);

		// Knights occupying central squares.
		score += ((w_knights & centrew).on_bits() - (b_knights & centreb).on_bits()) * centre_score;
		score += ((w_knights & advancedw).on_bits() - (b_knights & advancedb).on_bits()) * advanced_score;
		return score;
	}

	int rook_score(Position &p)
	{
		int score = 0;
		int on_open_file = 33;
		auto w_rooks = p.get_piece_bitboard(Colour::WHITE, Piece::ROOK);
		auto w_occup = p.occupants(Colour::WHITE);
		auto b_rooks = p.get_piece_bitboard(Colour::BLACK, Piece::ROOK);
		auto b_occup = p.occupants(Colour::BLACK);

		// Rooks on open files, big score for doubled on open files.
		auto files = Bitboard::get_files();
		for (int file = Bitboard::A_FILE; file < Bitboard::NUMBER_FILES; file++)
		{
			bool wopenf = (files.data[file] & (w_occup ^ w_rooks)).on_bits() == 0;
			bool bopenf = (files.data[file] & (b_occup ^ b_rooks)).on_bits() == 0;
			if (wopenf)
				score += pow((w_rooks & files.data[file]).on_bits(),2) * on_open_file;
			if (bopenf)
				score -= pow((b_rooks & files.data[file]).on_bits(),2) * on_open_file;
		}

		return score;
	}

	int bishop_score(Position &p)
	{
		int score = 0;
		int good_bishop_score = 40;
		
		auto w_bishops = p.get_piece_bitboard(Colour::WHITE, Piece::BISHOP);
		auto b_bishops = p.get_piece_bitboard(Colour::BLACK, Piece::BISHOP);
		auto occupants = p.occupants();

		// Bishops on long diagonals
		for (auto sq : w_bishops.non_empty_squares())
		{
			score += Bitboard::bishop_attacks(occupants, sq).on_bits() > 4 ? good_bishop_score : 0;
		}
		for (auto sq : w_bishops.non_empty_squares())
		{
			score -= Bitboard::bishop_attacks(occupants, sq).on_bits() > 4 ? good_bishop_score : 0;
		}

		return score;
	}

	int static_score(Position &p)
	{
		// Material score
		int score = 0;
		score += material_score(p);

		// King score
		score += king_score(p);
		
		// Pawn score
		score += pawn_score(p);

		// Rook score
		score += rook_score(p);

		// Knight score
		score += knight_score(p);

		// Bishop score
		score += bishop_score(p);

		return score;
	}
};