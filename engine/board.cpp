#include "board.h"
#include <assert.h>
namespace medusa
{
	// Directional attacks for bishops, queens and rooks.
	Bitboard direction_attacks(Bitboard occupants, Square sqr, const std::pair<int, int> *directions)
	{
		Bitboard attacked;
		for (int i = 0; i < 4; i++)
		{
			auto dir = directions[i];
			int sidx = static_cast<int>(sqr);
			while (true)
			{
				int old_sidx = sidx;
				sidx += 8*dir.first + dir.second;
				// Out of bounds, break.
				if (sidx < 0 || sidx > 63)
					break;
				// New square is not neighbouring the old one, break.
				if(!(squares[sidx] & neighbours[old_sidx]))
					break;
				// New square is attacked
				attacked = attacked | squares[sidx];
				// Bumped into an occupant, square is attacked (defended), break.
				if (squares[sidx] & occupants)
					break;
			}
		}

		return attacked;
	}

	// Square functions
	// ----------------
	// Move a square
	Bitboard bit_move(Bitboard bb, Square from, Square to)
	{
		return on_bit(off_bit(bb, from), to);
	}
	// Check if a square is on 
	bool is_on(Bitboard bb, Square square)
	{
		Bitboard s = sqrbb(square);
		return bool(bb & s);
	}
	// Turn a square off
	Bitboard off_bit(Bitboard bb, Square off)
	{
		Bitboard s = sqrbb(off);
		return bb & (~s);
	}
	// Turn a square on
	Bitboard on_bit(Bitboard bb, Square on)
	{
		Bitboard s = sqrbb(on);
		return bb | s;
	}
	// Convert bitboard to square
	Square bbsqr(Bitboard bb)
	{
		return static_cast<Square>(bb.msb_n());
	}
	// Convert square to bitboard
	Bitboard sqrbb(Square sqr)
	{
		return squares[sqr];
	}
	// Reflect a square
	Square reflect(Square sqr)
	{
		int square_int = static_cast<int>(sqr);
		square_int = (square_int % 8) + (7 - (square_int / 8)) * 8;
		Square output = static_cast<Square>(square_int);
		return output;
	}

	// Move creation.
	Move create_move(Square from, Square to)
	{
		return from + (to << 6);
	}

	Move create_promotion(Square from, Square to, Piece promo)
	{
		return create_move(from, to) + (PROMOTE << flag_bits) + (promo << prom_bits);
	}

	Move create_en_passant(Square from, Square to)
	{
		return create_move(from, to) + (CAPTURE_ENPASSANT << flag_bits);
	}

	// From and to will be the king, this will give information
	// about kingside/queenside and which king is castling.
	Move create_castle(Square from, Square to)
	{
		return create_move(from, to) + (CASTLE << flag_bits);
	}

	Square from(Move move)
	{
		const __int16 sqr = from_mask & move;
		return Square(sqr);
	}

	Square to(Move move)
	{
		return Square((to_mask & move) >> to_bits);
	}

	SpecialMove special_move(Move move)
	{
		return SpecialMove((flag_mask & move) >> flag_bits);
	}

	Piece promotion_piece(Move move)
	{
		return Piece((prom_mask & move) >> prom_bits);
	}

	std::string as_uci(Move move)
	{
		std::stringstream ss;
		ss << square_name(from(move));
		ss << square_name(to(move));
		if (special_move(move) == PROMOTE)
			ss << piece_string_lower(promotion_piece(move));
		return  ss.str();
	}

	Move reflect_move(Move move)
	{
		auto s = from(move);
		s = reflect(s);

		auto f = to(move);
		f = reflect(f);

		return s + (f << to_bits) + ((flag_mask + prom_mask) & move);
	}

	// Variadic template way of getting several squares
	// Get square
	Bitboard get_sq(medusa::Square sqr)
	{
		return sqrbb(sqr);
	}

	// Board geometry functions	
	// ------------------------
	// Reflect bitboard
	Bitboard reflect(Bitboard bb)
	{
		return	((bb & ranks[0]) << 56) |
			((bb & ranks[1]) << 40) |
			((bb & ranks[2]) << 24) |
			((bb & ranks[3]) << 8) |
			((bb & ranks[4]) >> 8) |
			((bb & ranks[5]) >> 24) |
			((bb & ranks[6]) >> 40) |
			((bb & ranks[7]) >> 56);
	}
}