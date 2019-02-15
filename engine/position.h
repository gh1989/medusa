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

		bool three_move_repetition() const
		{
			return false;
		}

		std::vector<Move> legal_moves();
		bool any_legal_move();

		Bitboard get_piece_bitboard(Colour colour, Piece piece) const
		{
			int index = colour.index();
			return bitboards[index][piece];
		}

		// Just want to deal with an enum in eval...
		Bitboard get_piece_bitboard(int index, Piece piece) const
		{
			return bitboards[index][piece];
		}

		std::vector<Move> pseudo_legal_moves();
		bool is_illegal_move(Move move, bool check_discovered_);

		template <Piece piece>
		std::vector<Move> legal_jumper_moves(
			Colour colour,
			const Bitboard* attacks) const;

		template <Piece piece>
		std::vector<Move> legal_slider_moves(
			Colour colour,
			const std::pair<int, int> *directions) const;

		std::vector<Move> legal_pawn_moves(Colour colour) const;
		std::vector<Move> legal_castling_moves(Colour colour) const;
		bool is_square_attacked(const Bitboard& square, Colour colour) const;
		void set_colour(Colour colour) { to_move = colour; }
		//void pp() const;
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

		static Position Pop()
		{
			Position ret = history.back();
			history.pop_back();
			return ret;
		}
	};


}

#endif
