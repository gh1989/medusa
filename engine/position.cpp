#include "position.h"
#include "utils.h"

#include <algorithm>
#include <iostream>

namespace medusa
{

	std::vector<Position> PositionHistory::history;

	void Position::apply(Move move)
	{
		PositionHistory::Push(*this);

		auto special_flag = special_move(move);
		auto us = colour_to_move();
		auto them = ~us;
		int idx = us.index();
		int them_idx = them.index();
		auto start = from(move);
		auto finish = to(move);
		bool reset50 = false;
		bool clear_enpassant = true;

		// Move piece from, no matter what.
		int p = 0;
		for (p = 0; p < NUMBER_PIECES; p++)
		{
			auto our_piece = bitboards[idx][p];
			if (is_on(our_piece, start))
			{
				if (p == PAWN)
					reset50 = true;

				if (p == PAWN && abs(finish - start) > 15)
				{
					Bitboard new_enpassant(1ULL << (us.is_white() ? (finish - 8) : (start - 8)));
					enpassant = new_enpassant;
					clear_enpassant = false;
				}

				if (p == KING) {
					castle_disable(us);
				}

				if (p == ROOK) {
					switch (start) {
					case(a1):
					{
						castle_disable(W_QUEENSIDE);
						break;
					}
					case(h1):
					{
						castle_disable(W_KINGSIDE);
						break;
					}
					case(a8):
					{
						castle_disable(B_QUEENSIDE);
						break;
					}
					case(h8):
					{
						castle_disable(B_KINGSIDE);
						break;
					}
					}
				}

				bitboards[idx][p] = off_bit(our_piece, start);
				break;
			}
		}

		// Kill piece on destination (capture)
		for (int p = 0; p < NUMBER_PIECES; p++) {
			auto their_pieces = bitboards[them_idx][p];
			if (is_on(their_pieces, finish))
			{
				bitboards[them_idx][p] = off_bit(their_pieces, finish);
				reset50 = true;
				break;
			}
		}

		// Special move related...
		switch (special_flag)
		{
		case(CAPTURE_ENPASSANT):
		{
			// TODO: implement white POV only, for now colour logic.
			reset50 = true;
			auto their_pawns = bitboards[them_idx][PAWN];
			auto their_enpassant_pawn = us.is_white() ? (enpassant >> 8) : (enpassant << 8);
			bitboards[them_idx][PAWN] = their_pawns & ~their_enpassant_pawn; //kill pawn
			auto our_pawns = bitboards[idx][PAWN];
			bitboards[idx][PAWN] = our_pawns | enpassant;
			break;
		}
		case(PROMOTE):
		{
			reset50 = true;
			auto promote_piece = promotion_piece(move);
			auto our_promote_pieces = bitboards[idx][promote_piece];
			bitboards[idx][promote_piece] = on_bit(our_promote_pieces, finish);
			break;
		}
		case(CASTLE):
		{
			auto our_king = bitboards[idx][KING];
			bitboards[idx][KING] = on_bit(our_king, finish);
			auto our_rooks = bitboards[idx][ROOK];
			bool queenside = (finish % 8) < 4;
			auto rook_from = Square(queenside ? finish - 2 : finish + 1);
			auto rook_to = Square(queenside ? finish + 1 : finish - 1);
			bitboards[idx][ROOK] = bit_move(our_rooks, rook_from, rook_to);
			castle_disable(us);
			break;
		}
		case(NONE):
		{
			auto our_piece = bitboards[idx][p];
			bitboards[idx][p] = on_bit(our_piece, finish);
		}
		}

		if (reset50)
			fifty_counter = 0;
		else
			fifty_counter++;

		if (clear_enpassant)
			enpassant = 0;

#ifdef _DEBUG
		tick_forward(as_uci(move));
#else
		tick_forward();
#endif
	}

	void Position::unapply(Move move)
	{
		auto previous = PositionHistory::Pop();

		castling = previous.castling;
		bitboards = previous.bitboards;
		enpassant = previous.enpassant;
		fifty_counter = previous.fifty_counter;

		tick_back();
	}

	bool Position::was_capture(Move move) const
	{
		auto to_sqr = to(move);
		auto to_sqr_bb = squares[to_sqr];
		if ((to_sqr_bb & occupants(colour_to_move())))
			return true;
		if (special_move(move) == CAPTURE_ENPASSANT)
			return true;
		return false;
	}

	bool Position::is_capture(Move move) const
	{
		auto to_sqr = to(move);
		auto to_sqr_bb = squares[to_sqr];
		if ((to_sqr_bb & occupants()))
			return true;
		if (special_move(move) == CAPTURE_ENPASSANT)
			return true;
		return false;
	}

	Piece Position::piece_at_square(Square square) const
	{
		auto bb = squares[square];
		for (auto p = 0; p < NUMBER_PIECES; p++)
		{
			auto piece_bb = bitboards[0][p] | bitboards[1][p];
			if ((bb & piece_bb))
				return Piece(p);
		}

		return NO_PIECE;
	}

	Piece Position::attacker(Move move) const
	{
		auto from_sqr = from(move);
		return piece_at_square(from_sqr);
	}

	Piece Position::captured(Move move) const
	{
		auto to_sqr = to(move);
		return piece_at_square(to_sqr);
	}

	Position Position::reflect() const
	{
		Position position(*this);

		// Flip position of the pieces but keep the colour the same.
		for (int p = 0; p < NUMBER_PIECES; p++)
		{
			position.bitboards[0][p] = medusa::reflect(bitboards[0][p]);
			position.bitboards[1][p] = medusa::reflect(bitboards[1][p]);
		}

		// We need to make sure this is necessary. Not sure it is.
		position.enpassant = medusa::reflect(enpassant);
		position.castling_reflect = !castling_reflect;
		return position;
	}

	// is square attacked by white pieces.
	bool Position::is_checkmate()
	{
		if (in_check())
			return !any_legal_move();
		return false;
	}

	// is square attacked by attacker pieces. Colour is attacking side.
	bool Position::is_square_attacked(const Bitboard& square, Colour colour) const
	{
		auto occupancy = occupants();
		auto bb = bitboards[colour.index()];
		
		auto sqr = bbsqr(square);
		if (knight_attacks[sqr] & bb[KNIGHT])
			return true;
		if (neighbours[sqr] & bb[KING])
			return true;
		if (direction_attacks(occupancy, sqr, bishop_directions) & (bb[BISHOP] | bb[QUEEN]))
			return true;
		if (direction_attacks(occupancy, sqr, rook_directions) & (bb[ROOK] | bb[QUEEN]))
			return true;

		bool reverse_pawn_attacks = colour.is_black();
		Bitboard new_pawn_square(square);
		if (!reverse_pawn_attacks)
			new_pawn_square = rotate180(new_pawn_square);

		auto pawn_s = bbsqr(new_pawn_square);
		auto pawn_bits = bb[PAWN];
		auto attacks = pawn_attacks[pawn_s];
		if (!reverse_pawn_attacks)
			attacks = rotate180(attacks);
		if (attacks & pawn_bits)
			return true;

		return false;
	}

	bool Position::in_check() const
	{
		Colour us = colour_to_move();
		Colour them = ~us;

		auto king = bitboards[us.index()][KING];
		bool is_check = is_square_attacked(king, them);

		return is_check;
	}	
}