#include <assert.h>

#include "board.h"
#include "utils.h"

namespace Medusa
{
	// Directional attacks for bishops, queens and rooks.
	Bitboard DirectionAttacks(Bitboard occupants, Square sqr, const std::pair<int, int> *directions)
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
	Bitboard BitMove(Bitboard bb, Square from, Square to)
	{
		return OnBit(OffBit(bb, from), to);
	}
	// Check if a square is on 
	bool IsOn(Bitboard bb, Square square)
	{
		Bitboard s = SqrBb(square);
		return bool(bb & s);
	}
	// Turn a square off
	Bitboard OffBit(Bitboard bb, Square off)
	{
		Bitboard s = SqrBb(off);
		return bb & (~s);
	}
	// Turn a square on
	Bitboard OnBit(Bitboard bb, Square on)
	{
		Bitboard s = SqrBb(on);
		return bb | s;
	}
	// Convert bitboard to square
	Square BbSqr(Bitboard bb)
	{
		return static_cast<Square>(bb.nMSB());
	}
	// Convert square to bitboard
	Bitboard SqrBb(Square sqr)
	{
		return squares[sqr];
	}
	// Reflect a square
	Square Reflect(Square sqr)
	{
		int square_int = static_cast<int>(sqr);
		square_int = (square_int % 8) + (7 - (square_int / 8)) * 8;
		Square output = static_cast<Square>(square_int);
		return output;
	}

	// Move creation.
	Move CreateMove(Square from, Square to)
	{
		return from + (to << 6);
	}

	Move CreatePromotion(Square from, Square to, Piece promo)
	{
		return CreateMove(from, to) + (PROMOTE << flag_bits) + (promo << prom_bits);
	}

	Move CreateEnPassant(Square from, Square to)
	{
		return CreateMove(from, to) + (CAPTURE_ENPASSANT << flag_bits);
	}

	// From and to will be the king, this will give information
	// about kingside/queenside and which king is castling.
	Move CreateCastle(Square from, Square to)
	{
		return CreateMove(from, to) + (CASTLE << flag_bits);
	}

	Square GetFrom(Move move)
	{
		const __int16 sqr = from_mask & move;
		return Square(sqr);
	}

	Square GetTo(Move move)
	{
		return Square((to_mask & move) >> to_bits);
	}

	SpecialMove SpecialMoveType(Move move)
	{
		return SpecialMove((flag_mask & move) >> flag_bits);
	}

	Piece PromotionPiece(Move move)
	{
		return Piece((prom_mask & move) >> prom_bits);
	}

	std::string AsUci(Move move)
	{
		std::stringstream ss;
		ss << SquareName(GetFrom(move));
		ss << SquareName(GetTo(move));
		if (SpecialMoveType(move) == PROMOTE)
			ss << PieceStringLower(PromotionPiece(move));
		return  ss.str();
	}

	Move ReflectMove(Move move)
	{
		auto s = GetFrom(move);
		s = Reflect(s);

		auto f = GetTo(move);
		f = Reflect(f);

		return s + (f << to_bits) + ((flag_mask + prom_mask) & move);
	}

	// Variadic template way of getting several squares
	// Get square
	Bitboard GetSquare(Medusa::Square sqr)
	{
		return SqrBb(sqr);
	}

	// Board geometry functions	
	// ------------------------
	// Reflect bitboard
	Bitboard Reflect(Bitboard bb)
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