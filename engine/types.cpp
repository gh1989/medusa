#include "types.h"

const Colour Colour::WHITE = Colour( 1);
const Colour Colour::BLACK = Colour(-1);

const std::string PieceNS::PIECE_STRINGS = "NBRQKP";
const std::string piece_strings[6] = { "n", "b", "r", "q", "k", "p" };

std::string piece_string_lower(Piece piece)
{
	return piece_strings[piece];
}