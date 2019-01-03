#include "position.h"
#include "utils.h"

#include <algorithm>
#include <iostream>

std::vector<Position> PositionHistory::history;

void Position::apply(MoveTiny move)
{
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
	for(int p=0;p<Piece::NUMBER_PIECES;p++)
	{
		auto our_piece = bitboards.data[idx].data[p];
		if (our_piece.is_on(start))
		{
			if (p == Piece::PAWN)
				reset50 = true;

			if (p == Piece::PAWN && abs(finish - start) > 15){
				Bitboard new_enpassant(1 << (us.is_white() ? (finish - 8) : (start - 8)));
				enpassant = new_enpassant;
				clear_enpassant = false;
			}

			if (p == Piece::KING){
				castle_disable(us);
			}

			if (p == Piece::ROOK){
				switch(start){
				case(Bitboard::a1):
				{
					castle_disable(Castling::W_QUEENSIDE);
					break;
				}
				case(Bitboard::h1):
				{
					castle_disable(Castling::W_KINGSIDE);
					break;
				}
				case(Bitboard::a8):
				{
					castle_disable(Castling::B_QUEENSIDE);
					break;
				}
				case(Bitboard::h8):
				{
					castle_disable(Castling::B_KINGSIDE);
					break;
				}
				}
			}

			bitboards.data[idx].data[p] = our_piece.off_bit(start);
			break;
		}
	}

	// Kill piece on destination (capture)
	for (int p = 0; p < Piece::NUMBER_PIECES; p++) {
		auto their_pieces = bitboards.data[them_idx].data[p];
		if (their_pieces.is_on(finish))
		{
			bitboards.data[them_idx].data[p] = their_pieces.off_bit(finish);
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
		auto their_pawns = bitboards.data[them_idx].data[Piece::PAWN];
		auto their_enpassant_pawn = us.is_white() ? (enpassant << 8) : (enpassant >> 8); 
		bitboards.data[them_idx].data[Piece::PAWN] = their_pawns & ~their_enpassant_pawn; //kill pawn
		break;
	}
	case(PROMOTE):
	{
		reset50 = true;
		auto promote_piece = promotion_piece(move);
		auto our_promote_pieces = bitboards.data[idx].data[promote_piece];
		bitboards.data[idx].data[promote_piece] = our_promote_pieces.on_bit(finish);
		break;
	}
	case(CASTLE):
	{
		auto our_king = bitboards.data[idx].data[Piece::KING];
		bitboards.data[idx].data[Piece::KING] = our_king.on_bit(finish);
		auto our_rooks = bitboards.data[idx].data[Piece::ROOK];
		bool queenside = (finish % 8) < 4;
		auto rook_from = Bitboard::Square(queenside ? finish - 2 : finish + 1);
		auto rook_to   = Bitboard::Square(queenside ? finish + 1 : finish - 1);
		bitboards.data[idx].data[Piece::ROOK] = our_rooks.off_on_bit(rook_from, rook_to);
		castle_disable(us);
		break;
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

	PositionHistory::Push(*this);
}

void Position::unapply(MoveTiny move)
{
	auto previous = PositionHistory::Pop();

	castling  = previous.castling;
	bitboards = previous.bitboards;
	enpassant = previous.enpassant;

	tick_back();
}

bool Position::is_capture(MoveTiny move) const
{
	auto to_sqr    = to(move);

	// TODO: fix this awkwardness with squares getting.
	auto to_sqr_bb = Bitboard::get_squares().data[to_sqr];
	// TODO: fix this awkwardness for bitboard checking.
	if ((to_sqr_bb & occupants()).any())
		return true;

	if (special_move(move) == CAPTURE_ENPASSANT)
		return true;
	return false;
}

Piece Position::piece_at_square(Bitboard::Square square) const
{
	// TODO: fix this awkwardness with squares getting.
	auto bb = Bitboard::get_squares().data[square];
	for (auto p=0; p < Piece::NUMBER_PIECES; p++)
	{
		auto piece_bb = bitboards.data[0].data[p] | bitboards.data[1].data[p];
		if ((bb & piece_bb).any())
			return Piece(p);
	}

	return Piece::NO_PIECE;
}

Piece Position::attacker(MoveTiny move) const
{
	auto from_sqr = from(move);
	return piece_at_square(from_sqr);
}

Piece Position::captured(MoveTiny move) const
{
	auto to_sqr = to(move);
	return piece_at_square(to_sqr);
}

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
	auto new_enpassant = enpassant.reflect();
    
    return Position(reflected_bitboards, plies, fifty_counter, castling, new_enpassant, to_move, !castling_reflect);
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

// Print the board as ASCII art.
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
	Bitboard ep = enpassant;

	if(ep.any())
	{
		std::cout<<"en-passant: ";
		for(auto sqr : ep.non_empty_squares())
			std::cout<<Bitboard::square_name(sqr);	
		std::cout<<std::endl;
	}
	std::cout<<"fiftycounter: "<<fifty_counter<<std::endl;
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

// Apply UCI move to the position.
void Position::apply_uci(std::string move_str)
{
	auto moves = legal_moves();

	for (auto m : moves)
	{
		auto this_move_str = as_uci(m);
		if (this_move_str == move_str)
		{
			apply(m);
			return;
		}
	}

	std::cout << "Warning: attempted to apply illegal move." << std::endl;
}