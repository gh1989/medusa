#include "position.h"
#include "utils.h"

#include <algorithm>
#include <iostream>

Position Position::reflect() const
{
	TPieceBBs reflected_bitboards;

    // Flip position of the pieces but keep the colour the same.
    for(int p=0; p < Piece::NUMBER_PIECES; p++)
    {
        reflected_bitboards.data[0].data[p] = bitboards.data[0].data[p].reflect();
        reflected_bitboards.data[1].data[p] = bitboards.data[1].data[p].reflect();
    }

    // We need to make sure this is necessary. Not sure it is.
    std::vector<Bitboard> new_enpassant_stack;
	for(auto it=enpassant_stack.begin(); it!=enpassant_stack.end(); it++ )
    {
        new_enpassant_stack.push_back(it->reflect());
    }

    return Position(reflected_bitboards, plies, fifty_stack, castling, new_enpassant_stack, to_move, !castling_reflect);
}

// is square attacked by white pieces.
bool Position::is_checkmate()
{
	if (in_check())
	{
		return !any_legal_move();
	}

	return false;
}

// is square attacked by attacker pieces.
bool Position::is_square_attacked(const Bitboard& square, Colour attackers) const
{
    auto occupancy	= occupants();
    auto attack_bb	= bitboards.data[attackers.index()];

    auto pawns		= attack_bb.data[Piece::PAWN];
    auto knights	= attack_bb.data[Piece::KNIGHT];
    auto bishops	= attack_bb.data[Piece::BISHOP];
    auto queens		= attack_bb.data[Piece::QUEEN];
    auto rooks		= attack_bb.data[Piece::ROOK];
    auto king		= attack_bb.data[Piece::KING];

    return Bitboard::is_square_attacked(
		occupancy, 
		square, 
		pawns, 
		bishops, 
		knights, 
		queens, 
		rooks, 
		king, 
		attackers.is_black());
}

bool Position::in_check() const
{
	Colour us   = colour_to_move();
	Colour them = ~us;
	   
	auto king = bitboards.data[us.index()].data[Piece::KING];
	bool is_check = is_square_attacked(king, them);

	return is_check;
}

void Position::pp() const
{
	// collect information
    std::string pos[8][8];
    for(auto i=0;i<8;i++)
		for(auto j=0;j<8;j++)
	    	pos[i][j] = ' ';
	
    for(auto clr: {Colour::WHITE, Colour::BLACK})
    {
    	auto clr_bitboard = bitboards.data[clr.index()];
		for(int i=0; i<Piece::NUMBER_PIECES; ++i)
		{	
			auto occup = clr_bitboard.data[i];
			Piece piece = static_cast<Piece>(i);
			for(auto sqr : occup.non_empty_squares())
			{
				int sqr_int = static_cast<int>(sqr);
				std::string modif = piece_string(piece, clr);
				pos[(7-int(sqr_int/8))%8][(sqr_int%8)] = modif;
			}
		}
    }
    
    // print out the board
    std::string baseline = "+---";
    for(auto j=0;j<7;j++)
    	baseline += "+---";
    baseline += "+\n";
    
    std::string output = baseline;
    for(auto i=0;i<8;i++)
    {
		for(auto j=0;j<8;j++)
			output += "| " + pos[i][j] + " ";
		output += "|\n";
		output += baseline;
	}	
	
	std::cout<<output;
	Bitboard ep = enpassant_stack.back();

	if(ep.any())
	{
		std::cout<<"en-passant: ";
		for(auto sqr : ep.non_empty_squares())
			std::cout<<Bitboard::square_name(sqr);	
		std::cout<<std::endl;
	}
	std::cout<<"fiftycounter: "<<fifty_stack.back()<<std::endl;
	int castlerights = castling;
	const std::string crights = "QKqk";
	std::cout<<"castlerights: "<<castlerights<< " ";	
	for(char c: crights)
	{
		if( castlerights % 2 )
			std::cout<<c;
		castlerights /= 2;
	}
	
	std::cout<<std::endl;
	auto clr = colour_to_move();
	std::cout<<"plies: "<<plies<<std::endl;		
	std::cout<<"colour to move: "<< clr.to_string() <<std::endl;		
}

void Position::apply_uci(std::string move_str)
{
	auto moves = legal_moves();

	for (auto m : moves)
	{
		auto this_move_str = m.as_uci();
		if (this_move_str == move_str)
		{
			m.apply(*this);
			return;
		}
	}

	std::cout << "Warning: attempted to apply illegal move." << std::endl;
}