#ifndef move_tiny_h
#define move_tiny_h

#include <sstream>

#include "bitboard.h"
#include "types.h"

// MoveTiny (16 bit int)
typedef unsigned short MoveTiny;

// FROM			     (6 bits)
constexpr unsigned short to_bits   = 6;
constexpr unsigned short flag_bits = 12;
constexpr unsigned short prom_bits = 14;

constexpr unsigned short from_mask = 63;
// TO				 (6 bits)
constexpr unsigned short to_mask   = 63 << to_bits;
// SPECIAL MOVE      (2 bits)
constexpr unsigned short flag_mask = 3 << flag_bits;
// PROMOTION PIECE   (2 bits)
constexpr unsigned short prom_mask = 3 << prom_bits;

typedef Bitboard::Square Sqr;

enum PromotionPiece
{
	KNIGHT = 0,
	BISHOP = 1,
	ROOK   = 2,
	QUEEN  = 3
};

enum SpecialMove
{
	NONE		      = 0,
	CAPTURE_ENPASSANT = 1,
	CASTLE   		  = 2,
	PROMOTE  		  = 3,
};

std::string piece_string_lower(Piece piece);
MoveTiny create_move(Sqr from, Sqr to);
MoveTiny create_promotion(Sqr from, Sqr to, PromotionPiece promo);
MoveTiny create_en_passant(Sqr from, Sqr to);
MoveTiny create_castle(Sqr from, Sqr to);
Sqr from(MoveTiny move);
Sqr to(MoveTiny move);
SpecialMove special_move(MoveTiny move);
Piece promotion_piece(MoveTiny move);
std::string as_uci(MoveTiny move);
MoveTiny reflect_move_tiny(MoveTiny move);

#endif