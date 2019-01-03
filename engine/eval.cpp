#include "eval.h"

namespace Eval
{
	int material_score(Position &p)
	{
		int score = 0;

		// Material
		score += PAWN_SCORE		* p.get_piece_bitboard(Colour::WHITE, Piece::PAWN	).on_bits();
		score -= PAWN_SCORE		* p.get_piece_bitboard(Colour::BLACK, Piece::PAWN	).on_bits();
		score += KNIGHT_SCORE	* p.get_piece_bitboard(Colour::WHITE, Piece::KNIGHT	).on_bits();
		score -= KNIGHT_SCORE	* p.get_piece_bitboard(Colour::BLACK, Piece::KNIGHT	).on_bits();
		score += ROOK_SCORE		* p.get_piece_bitboard(Colour::WHITE, Piece::ROOK	).on_bits();
		score -= ROOK_SCORE		* p.get_piece_bitboard(Colour::BLACK, Piece::ROOK	).on_bits();
		score += BISHOP_SCORE	* p.get_piece_bitboard(Colour::WHITE, Piece::BISHOP	).on_bits();
		score -= BISHOP_SCORE	* p.get_piece_bitboard(Colour::BLACK, Piece::BISHOP	).on_bits();
		score += QUEEN_SCORE	* p.get_piece_bitboard(Colour::WHITE, Piece::QUEEN	).on_bits();
		score -= QUEEN_SCORE	* p.get_piece_bitboard(Colour::BLACK, Piece::QUEEN	).on_bits();

		return score;
	}

	int static_score(Position &p)
	{
		int score = 0;
		score += material_score(p);
		return score;
	}
};