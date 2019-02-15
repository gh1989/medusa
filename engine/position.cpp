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

	bool files_eq(Square x, Square y)
	{
		return ((x % 8) == (y % 8));
	}

	bool ranks_eq(Square x, Square y)
	{
		return ((x / 8) == (y / 8));
	}

	bool diag_to(Square a, Square b)
	{
		int dy = (a / 8) - (b / 8);
		int dx = (a % 8) - (b % 8);
		if (dy*dx > 0)
			return dx == dy;
		return false;
	}

	bool anti_diag_to(Square a, Square b)
	{
		int dy = (a / 8) - (b / 8);
		int dx = (a % 8) - (b % 8);
		if (dy*dx < 0)
			return abs(dx) == abs(dy);
		return false;
	}


	std::vector<Move> Position::legal_moves()
	{
		auto psuedo_legal = pseudo_legal_moves();
		bool check_d = in_check();
		auto pred = [this, check_d](Move move) {return this->is_illegal_move(move, check_d);  };
		auto to_remove = std::remove_if(psuedo_legal.begin(), psuedo_legal.end(), pred);
		psuedo_legal.erase(to_remove, psuedo_legal.end());
		return psuedo_legal;
	}

	bool Position::any_legal_move()
	{
		auto psuedo_legal = pseudo_legal_moves();
		bool check_d = in_check();
		auto pred = [this, check_d](Move move) {return !this->is_illegal_move(move, check_d);  };
		return std::any_of(psuedo_legal.begin(), psuedo_legal.end(), pred);
	}

	bool Position::is_illegal_move(Move move, bool check_discovered_)
	{
		// Now filter out all illegals due to check at the end.
		Colour us = to_move;
		Colour them = ~to_move;

		auto king = this->get_piece_bitboard(us, KING);
		bool check_discovered = check_discovered_;

		auto their_rooks = this->get_piece_bitboard(them, ROOK);
		auto their_queens = this->get_piece_bitboard(them, QUEEN);
		auto their_bishops = this->get_piece_bitboard(them, BISHOP);

		if (this->attacker(move) == KING)
			check_discovered = true;
		else
		{
			auto kingsq = bbsqr(king);
			auto start = from(move);
			auto finish = to(move);
			int fidx = kingsq % 8;
			int ridx = kingsq / 8;
			if (files_eq(kingsq, start) && !files_eq(start, finish))
			{
				auto kingfile = files[fidx];
				if (((their_rooks | their_queens) & kingfile))
					check_discovered = true;
			}
			if (ranks_eq(kingsq, start) && !ranks_eq(start, finish))
			{
				auto kingrank = ranks[ridx];
				if (((their_rooks | their_queens) & kingrank))
					check_discovered = true;
			}
			if (diag_to(kingsq, start) && !diag_to(start, finish))
			{
				auto kingdiag = diagonals[kingsq];
				if (((their_bishops | their_queens) & kingdiag))
					check_discovered = true;
			}
			if (anti_diag_to(kingsq, start) && !anti_diag_to(start, finish))
			{
				auto kingdiag = antidiagonals[kingsq];
				if (((their_bishops | their_queens) & kingdiag))
					check_discovered = true;
			}
		}

		if (check_discovered)
		{
			apply(move);
			auto king = this->bitboards[us.index()][KING];
			bool discovered_check = this->is_square_attacked(king, them);
			unapply(move);
			return discovered_check;
		}

		return false;
	};

	std::vector<Move> Position::pseudo_legal_moves()
	{
		Colour us = to_move;
		std::vector<Move> moves;

		// There are not going to be more than 64 legal moves in a standard game.
		moves.reserve(64);

		// Get all moves
		auto rook_moves = legal_slider_moves<ROOK>(us, rook_directions);
		auto bishop_moves = legal_slider_moves<BISHOP>(us, bishop_directions);
		auto queen_1 = legal_slider_moves<QUEEN>(us, bishop_directions);
		auto queen_2 = legal_slider_moves<QUEEN>(us, rook_directions);
		auto knight_moves = legal_jumper_moves<KNIGHT>(us, knight_attacks);
		auto king_moves = legal_jumper_moves<KING>(us, neighbours);
		auto king_castling = legal_castling_moves(us);
		auto pawn_moves = legal_pawn_moves(us);

		// Insert them all at the end
		moves.insert(moves.end(), pawn_moves.begin(), pawn_moves.end());
		moves.insert(moves.end(), rook_moves.begin(), rook_moves.end());
		moves.insert(moves.end(), bishop_moves.begin(), bishop_moves.end());
		moves.insert(moves.end(), queen_1.begin(), queen_1.end());
		moves.insert(moves.end(), queen_2.begin(), queen_2.end());
		moves.insert(moves.end(), king_moves.begin(), king_moves.end());
		moves.insert(moves.end(), knight_moves.begin(), knight_moves.end());
		moves.insert(moves.end(), king_castling.begin(), king_castling.end());

		return moves;
	}

	template <Piece piece>
	std::vector<Move> Position::legal_jumper_moves(
		Colour us,
		const Bitboard* attacks) const
	{
		std::vector<Move> moves;
		Bitboard piecebb = bitboards[us.index()][piece];
		Bitboard ours = occupants(us);
		for (int sqr = a1; sqr <= h8; sqr++)
		{
			Bitboard square = squares[sqr];
			if (!(square & piecebb))
				continue;
			auto attack = attacks[sqr];
			for (int sqr2 = a1; sqr2 <= h8; sqr2++)
			{
				if (!(attack & squares[sqr2]))
					continue;
				if (squares[sqr2] & ours)
					continue;
				moves.emplace_back(create_move(Square(sqr), Square(sqr2)));
			}
		}
		return moves;
	}

	template <Piece piece>
	std::vector<Move> Position::legal_slider_moves(Colour us, const std::pair<int, int> *directions) const
	{
		std::vector<Move> moves;
		Colour them = ~us;
		Bitboard piecebb = bitboards[us.index()][piece];
		Bitboard theirs = occupants(them);
		Bitboard ours = occupants(us);
		for (int sqr = a1; sqr <= h8; sqr++)
		{
			Bitboard square = squares[sqr];
			if (!(square & piecebb))
				continue;

			for (int i = 0; i < 4; i++)
			{
				auto dir = directions[i];
				int sidx = sqr;
				while (true)
				{
					int rank = sidx / 8;
					if (dir.first > 0 && rank == 7)
						break;
					if (dir.first < 0 && rank == 0)
						break;
					int file = sidx % 8;
					if (dir.second > 0 && file == 7)
						break;
					if (dir.second < 0 && file == 0)
						break;

					sidx += 8 * dir.first + dir.second;
					if (sidx > 63 | sidx < 0)
						break;
					Bitboard new_bb = squares[sidx];
					if (new_bb&ours)
						break;
					moves.emplace_back(create_move(Square(sqr), Square(sidx)));
					if (new_bb&theirs)
						break;
				}
			}
		}

		return moves;
	}

	std::vector<Move> Position::legal_pawn_moves(Colour colour) const
	{
		// Work out on the pawn moves based on the reflected position.
		Position context(*this);
		if (colour.is_black())
			context = context.reflect();

		// Promote pieces
		const Piece promote_pieces[] = {
			KNIGHT,
			BISHOP,
			ROOK,
			QUEEN
		};

		std::vector<Move> moves;
		moves.reserve(64);

		// Get the position information from the potentially reflected position.
		Bitboard pawns = context.bitboards[colour.index()][PAWN];
		Bitboard occ = context.occupants();
		Bitboard bocc = context.occupants(~colour);

		for (Square sqr = a1; sqr <= h8; sqr = Square(sqr + 1))
		{
			int rank = sqr / 8;
			Bitboard square = squares[sqr];
			if (!(square & pawns))
				continue;

			// Pushes
			Bitboard push_once = square << 8;
			if (!(push_once & occ))
			{
				if (rank == 1) // 2nd rank, double push.
				{
					Bitboard push_twice = square << 16;
					if (!(push_twice & occ))
						moves.emplace_back(create_move(sqr, Square(sqr + 16)));
				}
				if (rank == 6) // 7th rank, promotion
					for (auto& prom : promote_pieces)
						moves.emplace_back(create_promotion(sqr, Square(sqr + 8), prom));
				else
					moves.emplace_back(create_move(sqr, Square(sqr + 8)));
			}

			// Captures
			if (pawn_attacks[sqr] & (bocc | enpassant))
			{
				for (int shft : {7, 9})
				{
					Bitboard capt_diag = square << shft;
					Square to = Square(sqr + shft);
					if (capt_diag & bocc)
					{
						if (rank == 6) // 7th rank, promotion
							for (auto& prom : promote_pieces)
								moves.emplace_back(create_promotion(sqr, to, prom));
						else
						{
							// Taking enpassant
							if (pawn_attacks[sqr] & enpassant)
								moves.emplace_back(create_en_passant(sqr, to));
							else
								moves.emplace_back(create_move(sqr, to));
						}
					}
				}
			}
		}

		// Generate moves on the reflected position for convenience, so return
		// the reflected moves to be correct.
		if (colour.is_black())
		{
			std::transform(
				moves.begin(),
				moves.end(),
				moves.begin(),
				[](Move move) { return reflect_move(move); });
		}

		return moves;
	}

	std::vector<Move> Position::legal_castling_moves(Colour colour) const
	{
		std::vector<Move> moves;

		bool black = colour.is_black();
		auto castling = get_castling();
		auto all_occupants = occupants();

		Colour them = ~colour;

		auto queenside = black ? B_QUEENSIDE : W_QUEENSIDE;
		auto kingside = black ? B_KINGSIDE : W_KINGSIDE;
		bool reverse_pawn_attacks = !black;
		bool checked = in_check();

		// queenside castling
		if (bool(castling & queenside) && !checked)
		{
			auto near_square = squares[black ? d8 : d1];
			auto extra_square = squares[black ? b8 : b1];
			auto far_square = squares[black ? c8 : c1];
			bool unobstructed = !(all_occupants & (near_square | far_square | extra_square));
			if (unobstructed)
			{
				// Want to delay this expensive stuff.
				unobstructed &= !is_square_attacked(near_square, them);
				unobstructed &= !is_square_attacked(far_square, them);
				if (unobstructed)
				{
					auto move = create_castle(black ? e8 : e1, black ? c8 : c1);
					moves.push_back(move);
				}
			}
		}

		// kingside castling
		if (bool(castling & kingside) && !checked)
		{
			auto near_square = squares[black ? f8 : f1];
			auto far_square = squares[black ? g8 : g1];
			bool unobstructed = !(all_occupants & (near_square | far_square));
			if (unobstructed)
			{
				unobstructed &= !is_square_attacked(near_square, them);
				unobstructed &= !is_square_attacked(far_square, them);
				if (unobstructed)
				{
					auto move = create_castle(black ? e8 : e1, black ? g8 : g1);
					moves.push_back(move);
				}
			}
		}

		return moves;
	}

}