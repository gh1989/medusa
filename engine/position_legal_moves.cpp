#include "position.h"

#include <algorithm>
#include <iostream>

bool files_eq(Bitboard::Square x, Bitboard::Square y)
{
	return ((x % 8) == (y % 8));
}

bool ranks_eq(Bitboard::Square x, Bitboard::Square y)
{
	return ( (x / 8) == (y / 8));
}

bool diag_to(Bitboard::Square a, Bitboard::Square b)
{
	int dy = (a / 8) - (b / 8); 
	int dx = (a % 8) - (b % 8);
	if (dy*dx > 0)  
		return dx == dy;
	return false;
}

bool anti_diag_to(Bitboard::Square a, Bitboard::Square b)
{
	int dy = (a / 8) - (b / 8);
	int dx = (a % 8) - (b % 8);
	if (dy*dx < 0)
		return abs(dx) == abs(dy);
	return false;
}

Bitboard::Square square(Bitboard bitboard)
{
	return bitboard.bit_length();
}

std::vector<MoveTiny> Position::pseudo_legal_moves()
{
	Colour us = to_move;
	std::vector<MoveTiny> moves;

	// Get all moves
	auto rook_moves = legal_slider_moves(us, Piece::ROOK, &Bitboard::rook_attacks);
	auto bishop_moves = legal_slider_moves(us, Piece::BISHOP, &Bitboard::bishop_attacks);
	auto queen_moves = legal_slider_moves(us, Piece::QUEEN, &Bitboard::queen_attacks);
	auto knight_moves = legal_jumper_moves(us, Piece::KNIGHT, Bitboard::get_knight_attacks());
	auto king_moves = legal_jumper_moves(us, Piece::KING, Bitboard::get_king_attacks());
	auto king_castling = legal_castling_moves(us);
	auto pawn_moves = legal_pawn_moves(us);

	// There are not going to be more than 64 legal moves in a standard game.
	moves.reserve(64);

	// Insert them all at the end
	moves.insert(moves.end(), pawn_moves.begin(), pawn_moves.end());
	moves.insert(moves.end(), rook_moves.begin(), rook_moves.end());
	moves.insert(moves.end(), bishop_moves.begin(), bishop_moves.end());
	moves.insert(moves.end(), queen_moves.begin(), queen_moves.end());
	moves.insert(moves.end(), king_moves.begin(), king_moves.end());
	moves.insert(moves.end(), knight_moves.begin(), knight_moves.end());
	moves.insert(moves.end(), king_castling.begin(), king_castling.end());

	return moves;
}

std::vector<MoveTiny> Position::legal_moves()
{
	auto psuedo_legal = pseudo_legal_moves();
	bool check_d = in_check();
	auto pred = [this, check_d](MoveTiny move) {return this->is_illegal_move(move, check_d);  };
	auto to_remove = std::remove_if(psuedo_legal.begin(), psuedo_legal.end(), pred);
	psuedo_legal.erase(to_remove, psuedo_legal.end());
	return psuedo_legal;
}

bool Position::any_legal_move()
{
	auto psuedo_legal = pseudo_legal_moves();
	bool check_d = in_check();
	auto pred = [this, check_d](MoveTiny move){return !this->is_illegal_move(move, check_d);  };
	return std::any_of(psuedo_legal.begin(), psuedo_legal.end(), pred);
}

bool Position::is_illegal_move(MoveTiny move, bool check_discovered_)
{
	// Now filter out all illegals due to check at the end.
	Colour us = to_move;
	Colour them = ~to_move;

	auto king = this->get_piece_bitboard(us, Piece::KING);
	bool check_discovered = check_discovered_;

	auto their_rooks = this->get_piece_bitboard(them, Piece::ROOK);
	auto their_queens = this->get_piece_bitboard(them, Piece::QUEEN);
	auto their_bishops = this->get_piece_bitboard(them, Piece::BISHOP);

	if (this->attacker(move) == Piece::KING)
		check_discovered = true;
	else
	{
		auto kingsq = square(king);
		auto start = from(move);
		auto finish = to(move);

		if (files_eq(kingsq, start) && !files_eq(start, finish))
		{
			auto kingfile = Bitboard::get_file_mask(kingsq);
			if (((their_rooks | their_queens) & kingfile ).any())
				check_discovered = true;
		}
		if (ranks_eq(kingsq, start) && !ranks_eq(start, finish))
		{
			auto kingrank = Bitboard::get_rank_mask(kingsq);
			if (((their_rooks | their_queens) & kingrank).any())
				check_discovered = true;
		}
		if (diag_to(kingsq, start) && !diag_to(start, finish))
		{
			auto kingdiag = Bitboard::get_diag_mask(kingsq);
			if (((their_bishops | their_queens) & kingdiag).any())
				check_discovered = true;
		}
		if (anti_diag_to(kingsq, start) && !anti_diag_to(start, finish))
		{
			auto kingadiag = Bitboard::get_anti_diag_mask(kingsq);
			if (((their_bishops | their_queens) & kingadiag).any())
				check_discovered = true;
		}
	}

	if (check_discovered)
	{
		apply(move);
		auto king = this->bitboards.data[us.index()].data[Piece::KING];
		bool discovered_check = this->is_square_attacked(king, them);
		unapply(move);
		return discovered_check;
	}

	return false;
};


std::vector<MoveTiny> Position::legal_jumper_moves(
    Colour colour,
    Piece piece,
    const w_array<Bitboard, Bitboard::NUMBER_SQUARES>& attacks) const
{
    std::vector<MoveTiny> moves;
	auto opposite_colour = ~colour;
    Bitboard potential_jumps;
    Bitboard potential_captures;
    Bitboard piece_occupancy = bitboards.data[colour.index()].data[piece];
    Bitboard piece_bitboard = bitboards.data[colour.index()].data[piece];

    // TODO: Have these method return by reference.
    auto opposite_pieces = bitboards.data[opposite_colour.index()];
    auto their_occupants = occupants(opposite_colour); 
    auto our_occupants = occupants(colour);
    auto occupants = our_occupants | their_occupants;
    auto squares = Bitboard::get_squares();

    for (auto start : piece_occupancy.non_empty_squares())
    {
        potential_jumps = attacks.data[start] & ~occupants;
        potential_captures = attacks.data[start] & their_occupants;

        for (auto finish : potential_jumps.non_empty_squares())
        {
            auto move = create_move(start, finish);
            moves.push_back(move);
        }

        for (auto finish : potential_captures.non_empty_squares())
        {
            auto move = create_move(start, finish);
            moves.push_back(move);
        }
    }

    return moves;
}

std::vector<MoveTiny> Position::legal_slider_moves(
	Colour colour, 
	Piece piece, 
    std::function<Bitboard(const Bitboard&, Bitboard::Square)> attacks_func) const
{
    std::vector<MoveTiny> moves;
	Colour opposite_colour = ~colour;

    // TODO: Have these method return by reference.
    auto opposite_pieces    = bitboards.data[opposite_colour.index()];
    auto our_occupants      = occupants(colour);
    auto piece_occupancy    = bitboards.data[colour.index()].data[piece];
    auto their_occupants    = occupants(opposite_colour); 
    auto occupants          = our_occupants | their_occupants;
    auto piece_bitboard     = bitboards.data[colour.index()].data[piece];
    
    auto ranks   = Bitboard::get_ranks();
    auto squares = Bitboard::get_squares();

    Bitboard b_attacks;
    Bitboard potential_jumps;
    Bitboard potential_captures;

    for(auto start : piece_occupancy.non_empty_squares())
    {
        b_attacks = attacks_func(occupants, start);
        potential_jumps = b_attacks & ~occupants;
        potential_captures = b_attacks & their_occupants;
        
        for (auto finish : potential_jumps.non_empty_squares())
        {
            auto move = create_move(start, finish);
            moves.push_back(move);
        }
        
        for (auto finish : potential_captures.non_empty_squares())
        {      
			auto move = create_move(start, finish);
            moves.push_back(move);
        }
    }

    return moves;
}

std::vector<MoveTiny> Position::legal_pawn_moves(Colour colour) const
{
    Position context(*this);
	auto opposite_colour = ~colour;
    if (colour.is_black())
    {
        context = context.reflect();
    }
    
    std::vector<MoveTiny> moves;
    Bitboard potential_jumps;
    Bitboard potential_captures;

    // TODO: Have these method return by reference.
    auto enpassant_      = context.enpassant;
    auto opposite_pieces = context.bitboards.data[opposite_colour.index()];
    auto our_occupants   = context.occupants(colour);
    auto piece_occupancy = context.bitboards.data[colour.index()].data[Piece::PAWN];
    auto their_occupants = context.occupants(opposite_colour); 
    auto piece_bitboard  = context.bitboards.data[colour.index()].data[Piece::PAWN];

    auto occupants = our_occupants | their_occupants;

    auto ranks = Bitboard::get_ranks();
    auto squares = Bitboard::get_squares();
	auto pawn_attacks = Bitboard::get_pawn_attacks();
	auto pawn_pushes = Bitboard::get_pawn_pushes();			
	auto promote_rank = ranks.data[Bitboard::RANK_8];
	auto promote_pieces = {Piece::KNIGHT, Piece::BISHOP, Piece::ROOK, Piece::QUEEN};

	for(auto start : piece_occupancy.non_empty_squares())
	{
		auto start_bitboard = squares.data[start];
		auto pushes = pawn_pushes.data[start];
		auto attacks = pawn_attacks.data[start];
	
		potential_jumps = pushes & (~occupants);
		potential_captures = attacks & (enpassant_ | (their_occupants & (~our_occupants)));

		//____________________________________________________
        // Pawn pushes
        //____________________________________________________
		for(auto finish : potential_jumps.non_empty_squares())
     	{
            // Checks
     		Bitboard finish_bitboard = squares.data[finish];

            // Pawn pushes: promotion
 			if ((promote_rank & finish_bitboard).any())
 			{
 				for (auto& pp : promote_pieces)
 				{
					auto move = create_promotion(start, finish, PromotionPiece(pp));
 					moves.push_back(move);
 				}
 			}

            // Pawn pushes: no promotion, including double push.
 			else
 			{
 				Bitboard square_ahead = start_bitboard << 8;
 				if ((occupants & (square_ahead | finish_bitboard)).empty())
 				{
					auto move = create_move(start, finish);
 					moves.push_back(move);
 				}
 			}
     	}

        //____________________________________________________
        // Pawn captures
        //____________________________________________________
	 	for(auto finish : potential_captures.non_empty_squares())
	 	{
			auto finish_bitboard = squares.data[finish];
			
			if ((enpassant_ & finish_bitboard).any())
			{
				auto move = create_en_passant(start, finish);
				moves.push_back(move);
			}
			else if ((promote_rank & finish_bitboard).any())
			{
				for (auto& pp : promote_pieces)
				{
					auto move = create_promotion(start, finish, PromotionPiece(pp));
					moves.push_back(move);
				}
			}
			else
			{
				auto move = create_move(start, finish);
				moves.push_back(move);
			}
	 	}
	}

    // Reflect back
    if (colour.is_black())
    {
        std::transform(
            moves.begin(), 
            moves.end(), 
            moves.begin(), 
            [](MoveTiny move){ return reflect_move_tiny(move); } );
    }

	return moves;
}

std::vector<MoveTiny> Position::legal_castling_moves(Colour colour) const
{
    std::vector<MoveTiny> moves;

    bool black         = colour.is_black();
    auto castling      = get_castling();
    auto squares       = Bitboard::get_squares();   
    auto all_occupants = occupants();
    
	Colour opposite_colour = ~colour;
    
    auto queenside = black ? Castling::B_QUEENSIDE : Castling::W_QUEENSIDE;
    auto kingside = black ? Castling::B_KINGSIDE : Castling::W_KINGSIDE; 
    bool reverse_pawn_attacks = !black;
    bool checked = in_check();

    // queenside castling
    if (bool(castling & queenside) && !checked)
    {
        auto near_square = squares.data[black ? Bitboard::d8 : Bitboard::d1];
		auto extra_square = squares.data[black ? Bitboard::b8 : Bitboard::b1];
        auto far_square = squares.data[black ? Bitboard::c8 : Bitboard::c1];
        bool unobstructed = !is_square_attacked(near_square, opposite_colour); 
        unobstructed &= !is_square_attacked(far_square, opposite_colour);  
        unobstructed &= !(all_occupants & (near_square | far_square | extra_square)).any();
        if (unobstructed)
        {
			auto move = create_castle(black ? Bitboard::e8 : Bitboard::e1, black ? Bitboard::c8 : Bitboard::c1);
            moves.push_back(move);
        }
    }

    // kingside castling
    if (bool(castling & kingside) && !checked)
    {
        auto near_square = squares.data[black ? Bitboard::f8 : Bitboard::f1];
        auto far_square = squares.data[black ? Bitboard::g8 : Bitboard::g1];
        bool unobstructed = !is_square_attacked(near_square, opposite_colour);   
        unobstructed &= !is_square_attacked(far_square, opposite_colour);  
        unobstructed &= !(all_occupants & (near_square | far_square)).any();
        if (unobstructed)
        {
			auto move = create_castle(black ? Bitboard::e8 : Bitboard::e1, black ? Bitboard::g8 : Bitboard::g1);
            moves.push_back(move);
        }
    }

    return moves;
}
