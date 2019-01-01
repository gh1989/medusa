#include "position.h"

#include <algorithm>
#include <iostream>
#include <stdint.h>
#include <sstream>
#include <string>     

void MoveImpl::board_string(Bitboard squares)
{
	std::cout<<std::endl;
	for(auto finish : squares.non_empty_squares())
		std::cout<<Bitboard::square_name(finish)<<",";
	std::cout<<std::endl;
}

std::string MoveImpl::piece_string(Piece piece)
{
	const std::string piece_strings[6] = {"P", "N", "B", "R", "Q", "K"};
	return piece_strings[piece];
}

std::string piece_string_lower(Piece piece)
{
	const std::string piece_strings[6] = { "p", "n", "b", "r", "q", "k" };
	return piece_strings[piece];
}

std::string MoveImpl::as_uci() const
{
	std::stringstream ss;
	ss << Bitboard::square_name(start);
	ss << Bitboard::square_name(finish);
	return  ss.str();
}

std::string PromotionMoveImpl::as_uci() const
{
	std::stringstream ss;
	ss << MoveImpl::as_uci();
	auto str = piece_string_lower(promote_piece);
	ss << str;
	return ss.str();
}

bool SimpleMoveImpl::described_by(std::string str) const
{
    auto finish_string = Bitboard::square_name(finish);
    auto start_string = Bitboard::square_name(start);
    
    auto msize = str.size();
    switch(piece)
    {
        case Piece::PAWN:
            return finish_string == str;
            break;
        default:
        {
            if ( msize<3 || msize>5 )
                return false;
            
            if(piece_string(piece) != str.substr(0,1))
                return false;
            
            auto fsquare = (msize == 3) ? str.substr(1,2) :
            (msize == 4) ? str.substr(2,2) :
            str.substr(3,2);
            
            if(fsquare != finish_string)
                return false;
            
            auto qualifier = (msize == 3) ? "" :
            (msize == 4) ? str.substr(1,1) :
            str.substr(1,2);
            
            if(qualifier.size()>0)
            {
                auto found = start_string.find(qualifier);
                if(found != std::string::npos)
                {
                    return true;
                }
                else return false;
            }
            else return true;
            break;
        }
    }
}

bool CaptureMoveImpl::described_by(std::string str) const
{
    auto finish_string = Bitboard::square_name(finish);
    auto start_string = Bitboard::square_name(start);

    auto msize = str.size();
    if(msize<4 || msize>6 || str[msize-3] != 'x')
        return false;
    
    switch(piece)
    {
        case Piece::PAWN:
        {
            if(msize != 4)
                return false;
            
            if(str[0] != start_string[0])
                return false;
            
            return finish_string == str.substr(2,2);
            break;
        }
        default:
        {
            if(piece_string(piece) != str.substr(0,1))
            {
                return false;
            }
            
            auto fsquare = (msize == 4) ? str.substr(2,2) :
            (msize == 5) ? str.substr(3,2) :
            str.substr(4,2);
            
            if(fsquare != finish_string)
            {
                return false;
            }
            auto qualifier = (msize == 4) ? "" :
            (msize == 5) ? str.substr(1,1) :
            str.substr(1,2);
            
            if(qualifier.size()>0)
            {
                auto found = start_string.find(qualifier);
                if(found != std::string::npos)
                {
                    return true;
                }
                else
                {
					return false;
                } 
                
            }
            else return true;
        }
    }
}

bool PromotionMoveImpl::described_by(std::string str) const
{
    auto msize = str.size();
    if(msize<3 || msize>4 ) // e8N and e8=N both accepted
        return false;
    
    auto finish_string = Bitboard::square_name(finish);
    
    auto piece_conv = str.substr(str.size()-1,1);
    if(piece_string(promote_piece) != piece_conv)
        return false;
    
    auto finish_square = str.substr(0,2);
    return finish_string == finish_square;
}

bool PromotionCaptureMoveImpl::described_by(std::string str) const
{
    auto finish_string = Bitboard::square_name(finish);
    auto start_string = Bitboard::square_name(start);
    
    auto msize = str.size();
    if(msize<5 || msize>6 || str[1] != 'x')
        return false;
    
    if(start_string[0] != str[0])
        return false;
    
    if(str.substr(2,2) != finish_string)
        return false;
    
    auto piece_char = piece_string(promote_piece);
    auto piece_char_comp = str.substr(str.size()-1,1);
    return piece_char_comp == piece_char;
}

bool CastleMoveImpl::described_by(std::string str) const
{
    return str == as_string();
}


std::string SimpleMoveImpl::as_string() const
{
	std::string p_string = piece_string(piece);
	std::string s_string = Bitboard::square_name(start);
	std::string f_string = Bitboard::square_name(finish);
	return p_string + s_string + f_string;
}

std::string CaptureMoveImpl::as_string() const
{
	std::string p_string = piece_string(piece);
	std::string s_string = Bitboard::square_name(start);
	std::string f_string = Bitboard::square_name(finish);
	const std::string captures = "x";
	return p_string + s_string + captures + f_string;
}

std::string PromotionMoveImpl::as_string() const
{
	std::string p_string = piece_string(promote_piece);
	std::string s_string = Bitboard::square_name(start);
	std::string f_string = Bitboard::square_name(finish);
	const std::string promotes_to = "="; 
	return "P" + s_string + f_string + promotes_to + p_string;
}

std::string PromotionCaptureMoveImpl::as_string() const
{
	std::string p_string = piece_string(promote_piece);
	std::string s_string = Bitboard::square_name(start);
	std::string f_string = Bitboard::square_name(finish);
	const std::string captures = "x";
	const std::string promotes_to = "=";
	return "P" + s_string + captures + f_string + promotes_to + p_string;
}

std::string CastleMoveImpl::as_string() const
{
	std::string str;
	switch (finish)
	{
		case(Bitboard::Square::c8):
		case(Bitboard::Square::c1):
			return "O-O-O";
		case(Bitboard::Square::g8):
		case(Bitboard::Square::g1):
			return "O-O";
		default:
			return "?";
	}
}

std::shared_ptr<MoveImpl> SimpleMoveImpl::reflect() const
{
	auto reflected_start = Bitboard::reflect(start);
	auto reflected_finish = Bitboard::reflect(finish);

	// Colour remains the same. Only geometric reflection.
	return std::shared_ptr<MoveImpl>(new SimpleMoveImpl(colour, piece, reflected_start, reflected_finish));
}

std::shared_ptr<MoveImpl> CaptureMoveImpl::reflect() const
{
	auto reflected_start = Bitboard::reflect(start);
	auto reflected_finish = Bitboard::reflect(finish);
	auto reflected_captured_start = Bitboard::reflect(captured_start);

	// Colour remains the same. Only geometric reflection.
	return std::shared_ptr<MoveImpl>(
		new CaptureMoveImpl(colour, piece, captured_piece, reflected_start, reflected_finish, reflected_captured_start));
}

std::shared_ptr<MoveImpl> PromotionMoveImpl::reflect() const
{
	auto reflected_start = Bitboard::reflect(start);
	auto reflected_finish = Bitboard::reflect(finish);

	// Colour remains the same. Only geometric reflection.
	return std::shared_ptr<MoveImpl>(
		new PromotionMoveImpl(colour, promote_piece, reflected_start, reflected_finish));
}


std::shared_ptr<MoveImpl> PromotionCaptureMoveImpl::reflect() const
{
	auto reflected_start = Bitboard::reflect(start);
	auto reflected_finish = Bitboard::reflect(finish);

	// Colour remains the same. Only geometric reflection.
	return std::shared_ptr<MoveImpl>(
		new PromotionCaptureMoveImpl(colour, promote_piece, captured_piece, reflected_start, reflected_finish));
}

std::shared_ptr<MoveImpl> CastleMoveImpl::reflect() const
{
	auto reflected_king_start = Bitboard::reflect(start);
	auto reflected_king_finish = Bitboard::reflect(finish);

	auto reflected_rook_start = Bitboard::reflect(rook_start);
	auto reflected_rook_finish = Bitboard::reflect(rook_finish);

	// Colour remains the same. Only geometric reflection.
	return std::shared_ptr<MoveImpl>(
		new CastleMoveImpl(colour, reflected_king_start, reflected_king_finish, reflected_rook_start, reflected_rook_finish));
}

void MoveImpl::apply(Position& pos) const
{
	castling_before = pos.get_castling();
#ifdef _DEBUG
	pos.tick_forward(this->as_string());
#else
	pos.tick_forward();
#endif
	
};

void MoveImpl::unapply(Position& pos) const
{
	pos.set_castling(castling_before);
	pos.tick_back();
};

void SimpleMoveImpl::apply(Position& pos) const
{
	MoveImpl::apply(pos);
	// Pawns clear the fifty move counter so push a 0 onto the
	// fifty move stack.
	if (piece == Piece::PAWN)
	{
		pos.push_fifty_clear();
		auto pawn_rank = colour.is_white() ? Bitboard::Rank::RANK_2 : Bitboard::Rank::RANK_7;
		auto d_push_rank = colour.is_white() ? Bitboard::Rank::RANK_4 : Bitboard::Rank::RANK_5;
		uint64_t bitnum = 2;
		bitnum = bitnum << (start-1);
		auto sqr = Bitboard(bitnum);

		if (Bitboard::in_rank(start, pawn_rank) && Bitboard::in_rank(finish, d_push_rank))
		{
			auto enpassant = (colour.is_white()) ? (sqr << 8) : (sqr >> 8);
			pos.push_enpassant(enpassant);
		}
		else
		{
			pos.push_enpassant_clear();	
		}
	}
	else
	{
		// Otherwise push n+1 onto the fifty move stack.
		pos.push_enpassant_clear();	
		pos.push_fifty_increment();
	}

	if (piece == Piece::KING)
	{
		pos.castle_disable(colour);
	}

	pos.move_piece(colour, piece, start, finish);

	if (piece == Piece::ROOK)
	{
		auto q_rook_square = colour.is_white() ? Bitboard::Square::a1 : Bitboard::Square::a8;
		auto k_rook_square = colour.is_white() ? Bitboard::Square::h1 : Bitboard::Square::h8;

		// If the rook is ever moved from the key squares then disable castling.
		if (start == q_rook_square)
		{
			auto castle_disable = colour.is_white() ? Castling::W_QUEENSIDE : Castling::B_QUEENSIDE;
			pos.castle_disable(castle_disable);
		}
		else if (start == k_rook_square)
		{
			auto castle_disable = colour.is_white() ? Castling::W_KINGSIDE : Castling::B_KINGSIDE;
			pos.castle_disable(castle_disable);
		}
	}

}

void SimpleMoveImpl::unapply(Position& pos) const
{
	pos.pop_fifty();
	pos.move_piece(colour, piece, finish, start);
	pos.pop_enpassant();
	MoveImpl::unapply(pos);
}

void CaptureMoveImpl::apply(Position& pos) const
{
	MoveImpl::apply(pos);


	// Capture their rook
	if (captured_piece == Piece::ROOK)
	{
		auto them = ~colour;
		auto their_queen_rook = them.is_black() ? Bitboard::Square::a8 : Bitboard::Square::a1;
		auto their_king_rook  = them.is_black() ? Bitboard::Square::h8 : Bitboard::Square::h1;

		if (finish == their_queen_rook )
		{
			auto castle_disable = them.is_black() ? Castling::B_QUEENSIDE : Castling::W_QUEENSIDE;
			pos.castle_disable(castle_disable);
		}
		else if (finish == their_king_rook)
		{
			auto castle_disable = them.is_black() ? Castling::B_KINGSIDE : Castling::W_KINGSIDE;
			pos.castle_disable(castle_disable);
		}
	}
	
	// Capture back with the rook.
	if (piece == Piece::ROOK)
	{
		auto our_queen_rook = colour.is_black() ? Bitboard::Square::a8 : Bitboard::Square::a1;
		auto our_king_rook  = colour.is_black() ? Bitboard::Square::h8 : Bitboard::Square::h1;

		if (start == our_queen_rook)
		{
			auto castle_disable = colour.is_black() ? Castling::B_QUEENSIDE : Castling::W_QUEENSIDE;
			pos.castle_disable(castle_disable);
		}
		else if (start == our_king_rook)
		{
			auto castle_disable = colour.is_black() ? Castling::B_KINGSIDE : Castling::W_KINGSIDE;
			pos.castle_disable(castle_disable);
		}
	}

	// Or if we move the king.
	if (piece == Piece::KING)
	{
		pos.castle_disable(colour);
	}

	pos.push_fifty_clear();		
	pos.remove_piece(opposite_colour, captured_piece, captured_start);
	pos.move_piece(colour, piece, start, finish);
	pos.push_enpassant_clear();
	
}

void CaptureMoveImpl::unapply(Position& pos) const
{
	pos.pop_fifty();
	pos.add_piece(opposite_colour, captured_piece, captured_start);
	pos.move_piece(colour, piece, finish, start);
	pos.pop_enpassant();
	MoveImpl::unapply(pos);
}

//-------------------------------------------------
// promotion move: apply: remove the pawn from start, add 
// the promotion piece to finish, push a zero fifty
// count onto the fifty stack and increase plies.
//-------------------------------------------------

void PromotionMoveImpl::apply(Position& pos) const
{
	MoveImpl::apply(pos);
	pos.remove_piece(colour, Piece::PAWN, start);
	pos.add_piece(colour, promote_piece, finish);
	pos.push_fifty_clear();
	pos.push_enpassant_clear();
	
}

//-------------------------------------------------
// promotion move: unapply: add the pawn to start, remove 
// the promotion piece at finish, pop the fifty stack
// and decrease plies.
//-------------------------------------------------

void PromotionMoveImpl::unapply(Position& pos) const
{
	pos.add_piece(colour, Piece::PAWN, start);
	pos.remove_piece(colour, promote_piece, finish);
	pos.pop_fifty();
	pos.pop_enpassant();
	MoveImpl::unapply(pos);
}


//-------------------------------------------------
// promotion move with capture: apply: remove the pawn 
// from start, remove the captured piece to the finish, add 
// the promotion piece to finish, push the fifty stack clear
// and increase plies. Also, if the queen or kingside
// rooks are taken then ensure that the castle stack is
// disable pushed.
//-------------------------------------------------

void PromotionCaptureMoveImpl::apply(Position& pos) const
{
	MoveImpl::apply(pos);
	pos.remove_piece(colour, Piece::PAWN, start);
	pos.remove_piece(opposite_colour, captured_piece, finish);
	pos.add_piece(colour, promote_piece, finish);
	pos.push_fifty_clear();

	// Reminder: colour checks, we are disabling THEIR castling.
	if (captured_piece == Piece::ROOK)
	{
		auto them = ~colour;
		auto q_rook_square = them.is_black() ? Bitboard::Square::a8 : Bitboard::Square::a1;
		auto k_rook_square = them.is_black() ? Bitboard::Square::h8 : Bitboard::Square::h1;

		if (finish == q_rook_square)
		{
			auto castle_disable = them.is_black() ? Castling::B_QUEENSIDE : Castling::W_QUEENSIDE;
			pos.castle_disable(castle_disable);
		}
		else if (finish == k_rook_square)
		{
			auto castle_disable = them.is_black() ? Castling::B_KINGSIDE : Castling::W_KINGSIDE;
			pos.castle_disable(castle_disable);
		}
	}

	pos.push_enpassant_clear();
}

//-------------------------------------------------
// promotion move with capture: unapply: add the pawn 
// to start, add the captured piece to the finish, remove 
// the promotion piece from finish, pop the fifty stack
// and decrease plies. Also, if the queen or kingside
// rooks were taken then ensure that the castle stack is
// popped.
//-------------------------------------------------

void PromotionCaptureMoveImpl::unapply(Position& pos) const
{
	pos.add_piece(colour, Piece::PAWN, start);
	pos.add_piece(opposite_colour, captured_piece, finish);
	pos.remove_piece(colour, promote_piece, finish);
	pos.pop_fifty();
	pos.pop_enpassant();	
	MoveImpl::unapply(pos);
}

void CastleMoveImpl::apply(Position& pos) const
{
	MoveImpl::apply(pos);
	pos.move_piece(colour, Piece::ROOK, rook_start, rook_finish);
	pos.move_piece(colour, Piece::KING, start, finish);
	pos.push_fifty_increment();
	pos.castle_disable(colour);
	pos.push_enpassant_clear();
}

void CastleMoveImpl::unapply(Position& pos) const
{
	pos.move_piece(colour, Piece::ROOK, rook_finish, rook_start);
	pos.move_piece(colour, Piece::KING, finish, start);
	pos.pop_fifty();
	pos.pop_enpassant();
	MoveImpl::unapply(pos);
}
