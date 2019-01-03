#include "utils.h"
#include "types.h"

#include <stdint.h>
#include <iostream>

std::string piece_string(Piece piece, Colour c)
{       
    auto str = PieceNS::PIECE_STRINGS[piece];
    if (c.is_black())
        str = ::tolower(str);
    return std::string(1, str);
}

Bitboard bitboard_from_string(std::string str)
{
	if(str[0] < 'a' || str[1] < '1' || str[0] > 'h' || str[1] > '8')
		throw std::runtime_error("Square string is formatted improperly.");
	uint64_t boardnum = str[0] - 'a' + 8*(str[1] - '1');
	return Bitboard(1ULL<<boardnum);
}

Position position_from_fen(std::string fen)
{
	const std::string castle_enum = "QKqk";
	
	if(fen == "")
        fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    
    std::vector<std::string> strings;
    std::istringstream f(fen);
    std::string s;

    while (std::getline(f, s, ' ')) {
        strings.push_back(s);
    }
    
    if(strings.size() != 6)
    {
    	throw std::runtime_error("FEN is formatted improperly.");
    }
    
	auto pos = Position(); // empty board
	auto posstring = strings[0];
	auto colour_string = strings[1];
	auto castlestring = strings[2];
	auto epstring = strings[3];
	auto fiftycounter = strings[4];
	auto moveclock = strings[5];
	
	auto ite = 8;
    unsigned short sub_ite,found;
    
    std::istringstream f2(posstring);
	strings.clear();
    while (std::getline(f2, s, '/')) {
    	sub_ite = 0;
		for(char& c : s) {
		    if(c >= '1' && c <= '9')
		    {
		    	sub_ite += c - '0';
		    }
		    else
		    {
				const std::string piece_strings = "NBRQKP";
		    	found = piece_strings.find(::toupper(c));
		    	if(found == std::string::npos)
		    	{
		    		throw std::runtime_error("FEN is formatted improperly.");
		    	}
		    	auto piece = static_cast<Piece>(found);
		    	auto colour = (::toupper(c) == c) ? Colour::WHITE : Colour::BLACK;
		    	auto square = static_cast<Bitboard::Square>(8*(ite-1)+sub_ite);
		    	pos.add_piece(colour,piece,square);
		    	sub_ite++;
		    }
		}
		if(sub_ite != 8)
		{
			throw std::runtime_error("FEN is formatted improperly.");
		}
		
        ite -= 1;
    }
    
    if(ite != 0)
    {
		throw std::runtime_error("FEN is formatted improperly.");    
	}
    
    if(castlestring != "-")
    {
		int castlesum = 0;
		for(char c: castlestring)
		{
			found = castle_enum.find(c);
			if( found != std::string::npos )
				castlesum += 1<<found;
		}
		
    	pos.set_castling(static_cast<Castling>(castlesum));
    }
    else
    {
    	pos.set_castling(static_cast<Castling>(0));
   	}
    
	auto colour = Colour::from_string(colour_string);
	pos.set_colour(colour);
    	
    if(epstring != "-")
    {
    	auto epboard = bitboard_from_string(epstring);
	    pos.set_enpassant(epboard);
	}
	else
	{
		pos.set_enpassant(0);
	}
	
	if(fiftycounter != "-")  
	{
		unsigned short counter = std::stoi(fiftycounter);
		if(counter >= 10000)
			throw std::runtime_error("FEN is formatted improperly.");
			
		pos.set_fifty_counter(counter);
	}
	else
	{
		pos.set_fifty_counter(0);
	}
		
	if(moveclock != "-")
	{
		unsigned short counter = (std::stoi(moveclock)-1);
		if(counter > 10000)
			throw std::runtime_error("FEN is formatted improperly.");
		
		pos.set_plies( 2*counter + colour.plies() );
	}
	
	return pos;
}

