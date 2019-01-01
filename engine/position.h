#ifndef position_h
#define position_h

#include "bitboard.h"
#include "move.h"
#include "types.h"

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
	    fifty_stack = position.fifty_stack;
	    plies = position.plies;
	    enpassant_stack = position.enpassant_stack;
		to_move = position.to_move;
		castling_reflect = position.castling_reflect;
	}

	Position(
		const TPieceBBs& bitboards_,
		unsigned short plies_,
		unsigned short fifty,
		const Castling& castling_,
		const Bitboard& enpassant,
		Colour to_move_,
		bool castling_reflect_)
	: 
		bitboards(bitboards_), 
		plies(plies_),
		to_move(to_move_),
		castling_reflect(castling_reflect_)
	{
		castling = castling_;
		enpassant_stack.push_back(enpassant);
		fifty_stack.push_back(fifty);
	}

	Position(
		const TPieceBBs& bitboards_,
		unsigned short plies_,
		const std::vector<unsigned short>& fifty_stack_,
		const Castling& castling_,
		const std::vector<Bitboard>& enpassant_stack_,
		Colour to_move_,
		bool castling_reflect_)
		:
		bitboards(bitboards_),
		plies(plies_),
		fifty_stack(fifty_stack_),
		castling(castling_),
		enpassant_stack(enpassant_stack_),
		to_move(to_move_),
		castling_reflect(castling_reflect_)
	{
	}

	Bitboard occupants() const
	{
		return occupants(Colour::WHITE) | occupants(Colour::BLACK);
	}

	Bitboard occupants(Colour colour) const
	{
		int index = colour.index();
		auto coloured_occupants_bitboards = bitboards.data[index];
		Bitboard ret(0);

		for(int i=0; i<Piece::NUMBER_PIECES; ++i)
		{
			ret = ret | coloured_occupants_bitboards.data[i];
		}

		return ret;
	}

    Position reflect() const;

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
	
	void push_fifty_clear()
	{
	    fifty_stack.push_back(0);
	}

	void push_fifty_increment()
	{
	    auto top = fifty_stack.back();
	    fifty_stack.push_back(++top);
	}

	void push_fifty(unsigned short top)
	{
	    fifty_stack.push_back(top);
	}
	
	unsigned short get_fifty_counter() const {return fifty_stack.back();}
	unsigned short get_plies() const {return plies;}
	
	Castling get_castling() const 
	{
		auto c = castling;
		if (castling_reflect)
			c = static_cast<Castling>((c % 4 << 2) + (c / 4));
		return c;
	} 
	void set_castling(Castling castling_) { castling = castling_; }

	Bitboard get_enpassant() const {return enpassant_stack.back();}

	void apply_uci(std::string move_str);
	
	void pop_fifty() 
	{
	    fifty_stack.pop_back();
	}
	
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

	w_array<int, Bitboard::NUMBER_SQUARES> board_control() const
	{
		// Now how important is each square on the board?
		auto importance = Bitboard::central_importance;

		// Get the actual attacks or each piece, black and white, then sum them
		auto white_pressure = attacks_pressure_matrix(Colour::WHITE);

		// The opposing king
		auto black_king = bitboards.data[0].data[Piece::KING];
		for (auto square : black_king.non_empty_squares())
		{
			auto attacks = Bitboard::get_king_attacks().data[square];
			importance = importance + attacks.to_pressure_matrix();
			importance = importance + black_king.to_pressure_matrix();
		}

		// Importance of the  white pressure
		white_pressure = white_pressure * importance;

		// Reset importance
		importance = Bitboard::central_importance;

		auto black_pressure = attacks_pressure_matrix(Colour::BLACK);
				
		auto white_king = bitboards.data[1].data[Piece::KING];
		for (auto square : white_king.non_empty_squares())
		{
			auto attacks = Bitboard::get_king_attacks().data[square];
			importance = importance + attacks.to_pressure_matrix();
			importance = importance + white_king.to_pressure_matrix();
		}

		black_pressure = black_pressure * importance;
		auto board_score = white_pressure - black_pressure;
		return board_score.clip_values(-1, 1);
	}

	w_array<int, Bitboard::NUMBER_SQUARES> attacks_pressure_matrix(Colour colour) const
	{
		auto occupancy = occupants();
		int index = colour.index();
		auto white_pieces = bitboards.data[index];
		auto king = white_pieces.data[Piece::KING];
		auto pawns = white_pieces.data[Piece::PAWN];
		auto knights = white_pieces.data[Piece::KNIGHT];
		auto bishops = white_pieces.data[Piece::BISHOP];
		auto queens = white_pieces.data[Piece::QUEEN];
		auto rooks = white_pieces.data[Piece::ROOK];
		auto pressure = Bitboard::attacks_pressure_matrix(occupancy, pawns, bishops, knights, queens, rooks, king, colour.is_black());
		return pressure;
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

	void push_enpassant(const Bitboard& bitboard_)
	{
		enpassant_stack.push_back(bitboard_);
	}

	void push_enpassant_clear()
	{
		enpassant_stack.push_back(Bitboard(0));
	}

	void pop_enpassant()
	{
		enpassant_stack.pop_back();
	}

    Colour colour_to_move() const
    {
		return to_move;
    }
 
	bool operator>=(const Position& other)
	{
		return this->operator>(other) || this->operator==(other);
	}

	bool operator<=(const Position& other)
	{
		return this->operator<(other) || this->operator==(other);
	}

	bool operator>(const Position& other)
	{
		for (auto p : { Piece::PAWN,   Piece::BISHOP, Piece::KING,
						Piece::KNIGHT, Piece::QUEEN,  Piece::ROOK })
		{
			if (bitboards.data[p].data[0].get_bit_number() <=
				other.bitboards.data[p].data[0].get_bit_number())
				return false;

			if (bitboards.data[p].data[1].get_bit_number() <=
				other.bitboards.data[p].data[1].get_bit_number())
				return false;
		}

		if (get_castling() <= other.get_castling())
		{
			return false;
		}

		if (get_enpassant().get_bit_number() <=
			other.get_enpassant().get_bit_number())
		{
			return false;
		}

		return true;
	}

	bool operator<(const Position& other)
	{
		for (auto p : { Piece::PAWN,   Piece::BISHOP, Piece::KING,
						Piece::KNIGHT, Piece::QUEEN,  Piece::ROOK })
		{
			if (bitboards.data[p].data[0].get_bit_number() >= 
				other.bitboards.data[p].data[0].get_bit_number())
				return false;

			if (bitboards.data[p].data[1].get_bit_number() >=
				other.bitboards.data[p].data[1].get_bit_number())
				return false;
		}

		if (get_castling() >= other.get_castling())
		{
			return false;
		}

		if (get_enpassant().get_bit_number() >=
			other.get_enpassant().get_bit_number())
		{
			return false;
		}

		return true;
	}

	bool operator==(const Position &other)
	{
		for (auto p : { Piece::PAWN,   Piece::BISHOP, Piece::KING,
					    Piece::KNIGHT, Piece::QUEEN,  Piece::ROOK })
		{
			if (!(bitboards.data[p].data[0] == other.bitboards.data[p].data[0]))
				return false;
			if (!(bitboards.data[p].data[1] == other.bitboards.data[p].data[1]))
				return false;
		}

		if (get_castling() != other.get_castling())
		{
			return false;
		}

		if (!(get_enpassant() == other.get_enpassant()))
		{
			return false;
		}

		return true;
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
	
	std::vector<Move> legal_moves();
	bool any_legal_move();

	Bitboard get_piece_bitboard(Colour colour, Piece piece) const
	{
		int index = colour.index();
		return bitboards.data[index].data[piece];
	}

	bool is_square_attacked(const Bitboard& square, Colour colour) const;
	void set_colour(Colour colour) { to_move = colour; }
	void pp() const;
	bool in_check() const;
    bool is_checkmate();

private:
	std::vector<Move> legal_pawn_moves(Colour colour) const;

	std::vector<Move> legal_slider_moves(
		Colour colour, 
		Piece piece, 
		std::function<Bitboard(const Bitboard&, Bitboard::Square)> attacks_func) const;

	std::vector<Move> legal_jumper_moves(
		Colour colour,
		Piece piece,
		const w_array<Bitboard, Bitboard::NUMBER_SQUARES>& attacks) const;

	std::vector<Move> legal_castling_moves(Colour colour) const;

    TPieceBBs bitboards;
    //std::vector<Castling> castle_stack;
	Castling castling;
    std::vector<unsigned short> fifty_stack;
    unsigned short plies;
    std::vector<Bitboard> enpassant_stack;
	std::vector<int> hash_stack;
	Colour to_move;
#ifdef _DEBUG
	std::vector<std::string> past_moves;
#endif
	mutable bool castling_reflect;
};

#endif
