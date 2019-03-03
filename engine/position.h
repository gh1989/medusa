#ifndef position_h
#define position_h

#include <array>
#include <functional>
#include <memory>
#include <vector>
#include <set>
#include <iostream>

#include "bitboard.h"
#include "board.h"
#include "types.h"

namespace medusa {

	enum MoveType
	{
		Any = 0,
		Capture = 1,
	};

	class Position
	{
	public:

		Position()
		{
			to_move = Colour::WHITE;
			plies = 0;
			castling = Castling::ALL;
			castling_reflect = false;
		}

		Position(
			const std::array<std::array<Bitboard, 6>, 2> bitboards_,
			unsigned short plies_,
			unsigned short fifty_counter_,
			Castling castling_,
			Bitboard enpassant_,
			Colour to_move_,
			bool castling_reflect_)
			:
			bitboards(bitboards_),
			plies(plies_),
			fifty_counter(fifty_counter_),
			castling(castling_),
			enpassant(enpassant_),
			to_move(to_move_),
			castling_reflect(castling_reflect_)
		{
		}

		void apply(Move move);
		void unapply(Move move);
		void pp() const;

		Bitboard occupants() const
		{
			return occupants(Colour::WHITE) | occupants(Colour::BLACK);
		}

		Bitboard occupants(Colour colour) const
		{
			int index = colour.index();
			return occupants(index);
		}

		Bitboard occupants(int index) const
		{
			auto coloured_occupants_bitboards = bitboards[index];
			Bitboard ret(0);

			for (int i = 0; i < Piece::NUMBER_PIECES; ++i)
			{
				ret = ret | coloured_occupants_bitboards[i];
			}

			return ret;
		}

		Position reflect() const;

		bool was_capture(Move move) const;

		void move_piece(Colour colour, Piece piece, Square start, Square finish)
		{
			int index = colour.index();
			auto piece_bitboard = bitboards[index][piece];
			if (!is_on(piece_bitboard, start) || is_on(piece_bitboard, finish))
			{
				throw("Tried to turn off/on bit which is on/off.");
			}

			piece_bitboard = bit_move(piece_bitboard, start, finish);
			bitboards[index][piece] = piece_bitboard;
		}

		void add_piece(Colour colour, Piece piece, Square square)
		{
			int index = colour.index();
			auto piece_bitboard = on_bit(bitboards[index][piece], square);
			bitboards[index][piece] = piece_bitboard;
		}

		void remove_piece(Colour colour, Piece piece, Square square)
		{
			int index = colour.index();
			auto piece_bitboard = off_bit(bitboards[index][piece], square);
			bitboards[index][piece] = piece_bitboard;
		}

		unsigned short get_fifty_counter() const { return fifty_counter; }
		unsigned short get_plies() const { return plies; }

		Castling get_castling() const
		{
			auto c = castling;
			if (castling_reflect)
				c = static_cast<Castling>((c % 4 << 2) + (c / 4));
			return c;
		}
		void set_castling(Castling castling_) { castling = castling_; }
		void set_fifty_counter(short fifty_counter_) { fifty_counter = fifty_counter_; }
		void set_enpassant(Bitboard enpassant_) { enpassant = enpassant_; }
		void apply_uci(std::string move_str);
		Piece piece_at_square(Square square) const;
		bool is_capture(Move move) const;
		Piece attacker(Move move) const;
		Piece captured(Move move) const;

		void tick_forward(
#ifdef _DEBUG
			std::string move_str
#endif
		)
		{
			to_move = ~to_move;
			plies++;
#ifdef _DEBUG
			past_moves.push_back(move_str);
#endif

		}

		void tick_back()
		{
			to_move = ~to_move;
			plies--;
#ifdef _DEBUG
			past_moves.pop_back();
#endif
		}

		void set_plies(int plies_)
		{
			plies = plies_;
		}

		void castle_disable(Colour colour)
		{
			Castling w_castling = static_cast<Castling>(Castling::W_QUEENSIDE | Castling::W_KINGSIDE);
			Castling b_castling = static_cast<Castling>(Castling::B_QUEENSIDE | Castling::B_KINGSIDE);
			Castling disable_this = colour.is_white() ? w_castling : b_castling;
			castle_disable(disable_this);
		}

		void castle_disable(Castling castle_disable)
		{
			castling = static_cast<Castling>(castling & ~castle_disable);
		}

		Colour colour_to_move() const
		{
			return to_move;
		}

		bool three_move_repetition() const;

		bool last_moved(Piece piece, Square square) const;

		bool operator==(const Position& other) const {
			bool equal = true;
			equal &= castling  == other.castling;
			equal &= enpassant == other.enpassant;
			equal &= to_move.index() == other.to_move.index();
			equal &= bitboards == other.bitboards;
			return equal;
		}

		bool operator!=(const Position& other) const { return !operator==(other); }

		template <MoveType MT>
		std::vector<Move> legal_moves()
		{
			auto psuedo_legal = pseudo_legal_moves<MT>();
			bool check_d = in_check();
			auto pred = [this, check_d](Move move) {return this->is_illegal_move(move, check_d);  };
			auto to_remove = std::remove_if(psuedo_legal.begin(), psuedo_legal.end(), pred);
			psuedo_legal.erase(to_remove, psuedo_legal.end());
			return psuedo_legal;
		}

		bool any_legal_move();

		Bitboard piecebb(Colour colour, Piece piece) const
		{
			int index = colour.index();
			return bitboards[index][piece];
		}

		// Just want to deal with an enum in eval...
		Bitboard piecebb(int index, Piece piece) const
		{
			return bitboards[index][piece];
		}

		template <MoveType MT>
		std::vector<Move> pseudo_legal_moves();
		bool is_illegal_move(Move move, bool check_discovered_);

		template <Piece piece, MoveType MT>
		std::vector<Move> legal_jumper_moves(
			Colour colour,
			const Bitboard* attacks) const;

		template <Piece piece, MoveType MT>
		std::vector<Move> legal_slider_moves(
			Colour colour,
			const std::pair<int, int> *directions) const;

		template <MoveType MT>
		std::vector<Move> legal_pawn_moves(Colour colour) const;

		template <MoveType MT>
		std::vector<Move> legal_castling_moves(Colour colour) const;
		bool is_square_attacked(const Bitboard& square, Colour colour) const;
		void set_colour(Colour colour) { to_move = colour; }
		bool in_check() const;
		bool is_checkmate();
		
	private:
		std::array<std::array<Bitboard, 6>, 2> bitboards;
		Castling castling;
		unsigned short fifty_counter;
		unsigned short plies;
		Bitboard enpassant;
		Colour to_move;

#ifdef _DEBUG
		std::vector<std::string> past_moves;
#endif
		mutable bool castling_reflect;
	};

	class PositionHistory
	{
	public:
		static std::vector<Position> history;

		static void Clear()
		{
			history.clear();
		}

		static void Push(const Position& position)
		{
			history.emplace_back(position);
		}

		// Was this piece moved to this square in the last move?
		static bool last_moved(Piece piece, Square square)
		{
			// Get the index of the history 2 plies ago.
			int idx = history.size() - 3;
			if (idx < 0)
				return false;

			// Get the piece bitboard from 4 plies ago.
			auto to_move = history.back().colour_to_move();
			auto piecebb = history[idx].piecebb(to_move, piece);

			// If we don't have the piece here then we moved it.
			return !(piecebb & sqrbb(square));
		}

		static Position Pop()
		{
			Position ret = history.back();
			history.pop_back();
			return ret;
		}

		static bool three_repetitions()
		{
			int idx = history.size() - 3;
			if (idx < 2)
				return false;

			const auto& last = history.back();
			
			// Is only a draw on the last position, can only be 
			// the same position if its the same players turn.
			int reps = 1;
			do 
			{
				if (history[idx] == last)
					reps++;
				idx = idx - 2;
			} while (idx > 0 && reps < 3);

			return reps >= 3;	
		}
	};

	inline bool Position::last_moved(Piece piece, Square square) const
	{
		// 2 plies ago was the piece still on this square?
		return PositionHistory::last_moved(piece, square);
	}

	inline bool Position::three_move_repetition() const
	{
		return PositionHistory::three_repetitions();
	}


	template <MoveType MT>
	inline std::vector<Move> Position::pseudo_legal_moves()
	{
		Colour us = to_move;
		std::vector<Move> moves;

		// There are not going to be more than 64 legal moves in a standard game.
		moves.reserve(64);

		// Get all moves
		auto rook_moves = legal_slider_moves<ROOK, MT>(us, rook_directions);
		auto bishop_moves = legal_slider_moves<BISHOP, MT>(us, bishop_directions);
		auto queen_1 = legal_slider_moves<QUEEN, MT>(us, bishop_directions);
		auto queen_2 = legal_slider_moves<QUEEN, MT>(us, rook_directions);
		auto knight_moves = legal_jumper_moves<KNIGHT, MT>(us, knight_attacks);
		auto king_moves = legal_jumper_moves<KING, MT>(us, neighbours);
		auto king_castling = legal_castling_moves<MT>(us);
		auto pawn_moves = legal_pawn_moves<MT>(us);

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


	template <Piece piece, MoveType MT>
	inline std::vector<Move> Position::legal_jumper_moves(
		Colour us,
		const Bitboard* attacks) const
	{
		std::vector<Move> moves;
		Bitboard piecebb = bitboards[us.index()][piece];
		Bitboard ours = occupants(us);
		Bitboard theirs = occupants(~us);
		for (auto it = piecebb.begin(); it != piecebb.end(); it.operator++())
		{
			Square sqr = Square(*it);
			;			Bitboard square = squares[sqr];
			auto attack = attacks[sqr] & (~ours);
			if (MT == Capture)
				attack &= theirs;
			for (auto it2 = attack.begin(); it2 != attack.end(); it2.operator++())
			{
				Square sqr2 = Square(*it2);
				auto move = create_move(Square(sqr), Square(sqr2));
				moves.emplace_back(move);
			}
		}
		return moves;
	}

	template <Piece piece, MoveType MT>
	inline std::vector<Move> Position::legal_slider_moves(Colour us, const std::pair<int, int> *directions) const
	{
		std::vector<Move> moves;
		Colour them = ~us;
		Bitboard piecebb = bitboards[us.index()][piece];
		Bitboard theirs = occupants(them);
		Bitboard ours = occupants(us);
		for (auto it = piecebb.begin(); it != piecebb.end(); it.operator++())
		{
			Square sqr = Square(*it);
			Bitboard square = squares[sqr];

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
					auto sqrto = Square(sidx);

					// Skip non-captures.
					auto move = create_move(Square(sqr), Square(sidx));
					if (MT == Capture && !(theirs & squares[sidx]))
						continue;

					moves.emplace_back(move);
					if (new_bb&theirs)
						break;
				}
			}
		}

		return moves;
	}

	template <MoveType MT>
	inline std::vector<Move> Position::legal_pawn_moves(Colour colour) const
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

		for (auto it = pawns.begin(); it != pawns.end(); it.operator++())
		{
			auto sqr = Square(*it);
			int rank = sqr / 8;
			Bitboard square = squares[sqr];

			// Pushes
			Bitboard push_once = square << 8;
			if (!(push_once & occ) && MT != Capture)
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
				for (int shft : {7, 9})
				{
					// You cannot capture off the side of the board.
					int file = sqr % 8;
					if (file == 0 && shft == 7)
						continue;
					if (file == 7 && shft == 9)
						continue;

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

	template <MoveType MT>
	inline std::vector<Move> Position::legal_castling_moves(Colour colour) const
	{
		std::vector<Move> moves;
		if (MT == Capture)
			return moves;

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

#endif
