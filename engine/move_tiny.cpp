#include "move_tiny.h"

// Tiny move creation.
MoveTiny create_move(Sqr from, Sqr to)
{
	return from + (to << 6);
}

MoveTiny create_promotion(Sqr from, Sqr to, PromotionPiece promo)
{
	return create_move(from, to) + (PROMOTE << flag_bits) + (promo << prom_bits);
}

MoveTiny create_en_passant(Sqr from, Sqr to)
{
	return create_move(from, to) + (CAPTURE_ENPASSANT << flag_bits);
}

// From and to will be the king, this will give information
// about kingside/queenside and which king is castling.
MoveTiny create_castle(Sqr from, Sqr to)
{
	return create_move(from, to) + (CASTLE << flag_bits);
}

Sqr from(MoveTiny move)
{
	const __int16 sqr = from_mask & move;
	return Sqr(sqr);
}

Sqr to(MoveTiny move)
{
	return Sqr((to_mask & move) >> to_bits);
}

SpecialMove special_move(MoveTiny move)
{
	return SpecialMove((flag_mask & move) >> flag_bits);
}

Piece promotion_piece(MoveTiny move)
{
	return Piece((prom_mask & move) >> prom_bits);
}

std::string as_uci(MoveTiny move)
{
	std::stringstream ss;
	ss << Bitboard::square_name(from(move));
	ss << Bitboard::square_name(to(move));
	if (special_move(move) == PROMOTE)
		ss << piece_string_lower(promotion_piece(move));
	return  ss.str();
}

MoveTiny reflect_move_tiny(MoveTiny move)
{
	auto s = from(move);
	s = Bitboard::reflect(s);

	auto f = to(move);
	f = Bitboard::reflect(f);

	return s + (f << to_bits) + ((flag_mask + prom_mask) & move);
}