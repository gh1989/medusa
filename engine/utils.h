#ifndef utils_h
#define utils_h

#include "move.h"
#include "position.h"

#include <exception>
#include <sstream>
#include <iostream>
#include <memory>
#include <string>

Position apply_from_string(Position& position, std::string str);
Move move_from_string(Position& position, std::string str);
std::string piece_string(Piece piece, Colour c);
Position position_from_fen(std::string str);
Bitboard bitboard_from_string(std::string str);
#endif
