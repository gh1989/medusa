#ifndef position_h
#define position_h

#include "bitboard.h"
#include "types.h"
#include "move_tiny.h"

#include <functional>
#include <memory>
#include <vector>
#include <set>
#include <iostream>

class Position
{
public:

	typedef w_array<w_array<Bitboard, Piece::NUMBER_PIECES>, 2> TPieceBBs;

	Position()
	{
		to_move = Colour::WHITE;
		plies = 0;
		castling = Castling::ALL;
		castling_reflect = false;
	}

	Position(const Position& position)
	{
		bitboards = position.bitboards;
		castling = position.castling;
	    fifty_counter = position.fifty_counter;
	    plies = position.plies;
	    enpassant = position.enpassant;
		to_move = position.to_move;
		castling_reflect = position.castling_reflect;
	}

	Position(
		const TPieceBBs& bitboards_,
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

	void apply(MoveTiny move);
	void unapply(MoveTiny move);

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
		auto coloured_occupants_bitboards = bitboards.data[index];
		Bitboard ret(0);

		for (int i = 0; i < Piece::NUMBER_PIECES; ++i)
		{
			ret = ret | coloured_occupants_bitboards.data[i];
		}

		return ret;
	}

    Position reflect() const;

	bool was_capture(MoveTiny move) const;

	void move_piece(Colour colour, Piece piece, Bitboard::Square start, Bitboard::Square finish)
	{
		int index = colour.index();
		auto piece_bitboard = bitboards.data[index].data[piece];
		if (!piece_bitboard.is_on(start) || piece_bitboard.is_on(finish))
		{
			throw("Tried to turn off/on bit which is on/off.");
		}

		piece_bitboard = piece_bitboard.off_on_bit(start, finish);
	    bitboards.data[index].data[piece] = piece_bitboard;
	}

	void add_piece(Colour colour, Piece piece, Bitboard::Square square)
	{
		int index = colour.index();
		auto piece_bitboard = bitboards.data[index].data[piece].on_bit(square);
	    bitboards.data[index].data[piece] = piece_bitboard;
	}

	void remove_piece(Colour colour, Piece piece, Bitboard::Square square)
	{
		int index = colour.index();
		auto piece_bitboard = bitboards.data[index].data[piece].off_bit(square);
	    bitboards.data[index].data[piece] = piece_bitboard;
	}
	
	unsigned short get_fifty_counter() const {return fifty_counter;}
	unsigned short get_plies() const {return plies;}
	
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
	Piece piece_at_square(Bitboard::Square square) const;
	bool is_capture(MoveTiny move) const;
	Piece attacker(MoveTiny move) const;
	Piece captured(MoveTiny move) const;

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
		/*
		int rep = 0;
		if (past_positions.size() < 3)
			return false;

		auto it = past_positions.begin();
		Position tmp = *it;
		it++;
		for(;it != past_positions.end(); ++it)
		{
			if (tmp == (*it))
				rep++;
			if (rep >= 3)
				break;
			tmp = *it;
		}

		return rep >= 3;
		*/
		return false;
	}
	
	std::vector<MoveTiny> legal_moves();
	bool any_legal_move();

	Bitboard get_piece_bitboard(Colour colour, Piece piece) const
	{
		int index = colour.index();
		return bitboards.data[index].data[piece];
	}

	// Just want to deal with an enum in eval...
	Bitboard get_piece_bitboard(int index, Piece piece) const
	{
		return bitboards.data[index].data[piece];
	}

	std::vector<MoveTiny> pseudo_legal_moves();
	bool is_illegal_move(MoveTiny move, bool check_discovered_);

	bool is_square_attacked(const Bitboard& square, Colour colour) const;
	void set_colour(Colour colour) { to_move = colour; }
	void pp() const;
	bool in_check() const;
    bool is_checkmate();


private:
	std::vector<MoveTiny> legal_pawn_moves(Colour colour) const;

	std::vector<MoveTiny> legal_slider_moves(
		Colour colour, 
		Piece piece, 
		std::function<Bitboard(const Bitboard&, Bitboard::Square)> attacks_func) const;

	std::vector<MoveTiny> legal_jumper_moves(
		Colour colour,
		Piece piece,
		const w_array<Bitboard, Bitboard::NUMBER_SQUARES>& attacks) const;

	std::vector<MoveTiny> legal_castling_moves(Colour colour) const;

    TPieceBBs bitboards;
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

#endif
