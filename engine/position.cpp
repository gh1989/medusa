#include "position.h"
#include "utils.h"

#include <algorithm>
#include <iostream>

namespace Medusa
{

	std::vector<Position> PositionHistory::history;

	void Position::Apply(Move move)
	{
		PositionHistory::Push(*this);

		auto special_flag = SpecialMoveType(move);
		auto us = ToMove();
		auto them = ~us;
		int idx = us.Index();
		int them_idx = them.Index();
		auto start = GetFrom(move);
		auto finish = GetTo(move);
		bool reset50 = false;
		bool clear_enpassant = true;

		// Move piece from, no matter what.
		int p = 0;
		for (p = 0; p < NUMBER_PIECES; p++)
		{
			auto our_piece = bitboards[idx][p];
			if (IsOn(our_piece, start))
			{
				if (p == PAWN)
					reset50 = true;

				if (p == PAWN && abs(finish - start) > 15)
				{
					Bitboard new_enpassant(1ULL << (us.IsWhite() ? (finish - 8) : (start - 8)));
					enpassant = new_enpassant;
					clear_enpassant = false;
				}

				if (p == KING) {
					DisableCastling(us);
				}

				if (p == ROOK) {
					switch (start) {
					case(a1):
					{
						DisableCastling(W_QUEENSIDE);
						break;
					}
					case(h1):
					{
						DisableCastling(W_KINGSIDE);
						break;
					}
					case(a8):
					{
						DisableCastling(B_QUEENSIDE);
						break;
					}
					case(h8):
					{
						DisableCastling(B_KINGSIDE);
						break;
					}
					}
				}

				bitboards[idx][p] = OffBit(our_piece, start);
				break;
			}
		}

		// Kill piece on destination (capture)
		for (int p = 0; p < NUMBER_PIECES; p++) {
			auto their_pieces = bitboards[them_idx][p];
			if (IsOn(their_pieces, finish))
			{
				bitboards[them_idx][p] = OffBit(their_pieces, finish);
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
			auto their_enpassant_pawn = us.IsWhite() ? (enpassant >> 8) : (enpassant << 8);
			bitboards[them_idx][PAWN] = their_pawns & ~their_enpassant_pawn; //kill pawn
			auto our_pawns = bitboards[idx][PAWN];
			bitboards[idx][PAWN] = our_pawns | enpassant;
			break;
		}
		case(PROMOTE):
		{
			reset50 = true;
			auto promote_piece = PromotionPiece(move);
			auto our_promote_pieces = bitboards[idx][promote_piece];
			bitboards[idx][promote_piece] = OnBit(our_promote_pieces, finish);
			break;
		}
		case(CASTLE):
		{
			auto our_king = bitboards[idx][KING];
			bitboards[idx][KING] = OnBit(our_king, finish);
			auto our_rooks = bitboards[idx][ROOK];
			bool queenside = (finish % 8) < 4;
			auto rook_from = Square(queenside ? finish - 2 : finish + 1);
			auto rook_to = Square(queenside ? finish + 1 : finish - 1);
			bitboards[idx][ROOK] = BitMove(our_rooks, rook_from, rook_to);
			DisableCastling(us);
			break;
		}
		case(NONE):
		{
			auto our_piece = bitboards[idx][p];
			bitboards[idx][p] = OnBit(our_piece, finish);
		}
		}

		if (reset50)
			fifty_counter = 0;
		else
			fifty_counter++;

		if (clear_enpassant)
			enpassant = 0;

		TickForward();
	}

	void Position::Unapply(Move move)
	{
		auto previous = PositionHistory::Pop();

		castling = previous.castling;
		bitboards = previous.bitboards;
		enpassant = previous.enpassant;
		fifty_counter = previous.fifty_counter;

		TickBack();
	}

	bool Position::MoveWasCapture(Move move) const
	{
		auto to_sqr = GetTo(move);
		auto to_sqr_bb = squares[to_sqr];
		if ((to_sqr_bb & Occupants(ToMove())))
			return true;
		if (SpecialMoveType(move) == CAPTURE_ENPASSANT)
			return true;
		return false;
	}

	bool Position::MoveIsCapture(Move move) const
	{
		auto to_sqr = GetTo(move);
		auto to_sqr_bb = squares[to_sqr];
		if ((to_sqr_bb & Occupants()))
			return true;
		if (SpecialMoveType(move) == CAPTURE_ENPASSANT)
			return true;
		return false;
	}

	Piece Position::PieceAtSquare(Square square) const
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

	Piece Position::GetAttacker(Move move) const
	{
		auto from_sqr = GetFrom(move);
		return PieceAtSquare(from_sqr);
	}

	Piece Position::Captured(Move move) const
	{
		auto to_sqr = GetTo(move);
		return PieceAtSquare(to_sqr);
	}

	Position Position::Reflect() const
	{
		Position position(*this);

		// Flip position of the pieces but keep the colour the same.
		for (int p = 0; p < NUMBER_PIECES; p++)
		{
			position.bitboards[0][p] = Medusa::Reflect(bitboards[0][p]);
			position.bitboards[1][p] = Medusa::Reflect(bitboards[1][p]);
		}

		// We need to make sure this is necessary. Not sure it is.
		position.enpassant = Medusa::Reflect(enpassant);
		position.castling_reflect = !castling_reflect;
		return position;
	}

	// is square attacked by white pieces.
	bool Position::IsCheckmate()
	{
		if (IsInCheck())
			return !AnyLegalMove();
		return false;
	}

	// is square attacked by attacker pieces. Colour is attacking side.
	bool Position::IsSquareAttacked(const Bitboard& square, Colour colour) const
	{
		auto occupancy = Occupants();
		auto bb = bitboards[colour.Index()];
		
		auto sqr = BbSqr(square);
		if (knight_attacks[sqr] & bb[KNIGHT])
			return true;
		if (neighbours[sqr] & bb[KING])
			return true;
		if (DirectionAttacks(occupancy, sqr, bishop_directions) & (bb[BISHOP] | bb[QUEEN]))
			return true;
		if (DirectionAttacks(occupancy, sqr, rook_directions) & (bb[ROOK] | bb[QUEEN]))
			return true;

		bool reverse_pawn_attacks = colour.IsBlack();
		Bitboard new_pawn_square(square);
		if (!reverse_pawn_attacks)
			new_pawn_square = Rotate180(new_pawn_square);

		auto pawn_s = BbSqr(new_pawn_square);
		auto pawn_bits = bb[PAWN];
		auto attacks = pawn_attacks[pawn_s];
		if (!reverse_pawn_attacks)
			attacks = Rotate180(attacks);
		if (attacks & pawn_bits)
			return true;

		return false;
	}

	bool Position::IsInCheck() const
	{
		Colour us = ToMove();
		Colour them = ~us;

		auto king = bitboards[us.Index()][KING];
		bool is_check = IsSquareAttacked(king, them);

		return is_check;
	}	

	bool FilesEqual(Square x, Square y)
	{
		return ((x % 8) == (y % 8));
	}

	bool RanksEqual(Square x, Square y)
	{
		return ((x / 8) == (y / 8));
	}

	bool DiagTo(Square a, Square b)
	{
		int dy = (a / 8) - (b / 8);
		int dx = (a % 8) - (b % 8);
		if (dy*dx > 0)
			return dx == dy;
		return false;
	}

	bool AntiDiagTo(Square a, Square b)
	{
		int dy = (a / 8) - (b / 8);
		int dx = (a % 8) - (b % 8);
		if (dy*dx < 0)
			return abs(dx) == abs(dy);
		return false;
	} 

	bool Position::AnyLegalMove()
	{
		auto psuedo_legal = PseudoLegalMoves<Any>();
		bool check_d = IsInCheck();
		auto pred = [this, check_d](Move move) {return !IsIllegalMove(move, check_d);  };
		return std::any_of(psuedo_legal.begin(), psuedo_legal.end(), pred);
	}

	bool Position::IsIllegalMove(Move move, bool check_discovered_)
	{
		// Now filter out all illegals due to check at the end.
		Colour us = to_move;
		Colour them = ~to_move;

		auto king = PieceBoard(us, KING);
		bool check_discovered = check_discovered_;

		auto their_rooks = PieceBoard(them, ROOK);
		auto their_queens = PieceBoard(them, QUEEN);
		auto their_bishops = PieceBoard(them, BISHOP);

		if (this->GetAttacker(move) == KING)
			check_discovered = true;
		else
		{
			auto kingsq = BbSqr(king);
			auto start = GetFrom(move);
			auto finish = GetTo(move);
			int fidx = kingsq % 8;
			int ridx = kingsq / 8;
			if (FilesEqual(kingsq, start) && !FilesEqual(start, finish))
			{
				auto kingfile = files[fidx];
				if (((their_rooks | their_queens) & kingfile))
					check_discovered = true;
			}
			if (RanksEqual(kingsq, start) && !RanksEqual(start, finish))
			{
				auto kingrank = ranks[ridx];
				if (((their_rooks | their_queens) & kingrank))
					check_discovered = true;
			}
			if (DiagTo(kingsq, start) && !DiagTo(start, finish))
			{
				auto kingdiag = diagonals[kingsq];
				if (((their_bishops | their_queens) & kingdiag))
					check_discovered = true;
			}
			if (AntiDiagTo(kingsq, start) && !AntiDiagTo(start, finish))
			{
				auto kingdiag = antidiagonals[kingsq];
				if (((their_bishops | their_queens) & kingdiag))
					check_discovered = true;
			}
		}

		if (check_discovered)
		{
			Apply(move);
			auto king = this->bitboards[us.Index()][KING];
			bool discovered_check = this->IsSquareAttacked(king, them);
			Unapply(move);
			return discovered_check;
		}

		return false;
	};
}