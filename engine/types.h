#ifndef types_h
#define types_h

#include <stdexcept>
#include <stdint.h>
#include <string>

namespace medusa
{
	class Colour
	{
	public:
		Colour() : value(1) {}
		Colour(int value_)
		{
			if (abs(value_) != 1)
				throw("Invalid colour");
			value = value_;
		}

		int operator*(int other) const {
			return value * other;
		}

		Colour operator~() const {
			return Colour(-value);
		}

		std::string to_string() const {
			return value < 0 ? "B" : "W";
		}

		bool is_black() const {
			return value < 0;
		}

		bool is_white() const {
			return !is_black();
		}

		int index() const {
			return (value < 0 ? 1 : 0);
		}

		int plies() const {
			return (value < 0 ? 1 : 0);
		}

		static Colour from_string(std::string colour_string) {
			if (colour_string == "w")
				return Colour::WHITE;
			if (colour_string == "b")
				return Colour::BLACK;
			throw("Colour string formatted improperly.");
		}

		static const Colour WHITE;
		static const Colour BLACK;

	private:
		int value;
	};

	static const std::string PIECE_STRINGS = "NBRQKP";

	enum Piece { KNIGHT, BISHOP, ROOK, QUEEN, KING, PAWN, NUMBER_PIECES, NO_PIECE = -1 };

	enum Castling {
		W_QUEENSIDE = 1,
		W_KINGSIDE = 2,
		B_QUEENSIDE = 4,
		B_KINGSIDE = 8,
		ALL = 15
	};

	enum Square
	{
		a1 = 0, b1, c1, d1, e1, f1, g1, h1,
		a2, b2, c2, d2, e2, f2, g2, h2,
		a3, b3, c3, d3, e3, f3, g3, h3,
		a4, b4, c4, d4, e4, f4, g4, h4,
		a5, b5, c5, d5, e5, f5, g5, h5,
		a6, b6, c6, d6, e6, f6, g6, h6,
		a7, b7, c7, d7, e7, f7, g7, h7,
		a8, b8, c8, d8, e8, f8, g8, h8,
	};

	std::string piece_string_lower(Piece piece);

	// Move functions
	// --------------
	// Move (16 bit int)
	typedef unsigned short Move;
	// From
	constexpr unsigned short to_bits = 6;
	constexpr unsigned short flag_bits = 12;
	constexpr unsigned short prom_bits = 14;
	constexpr unsigned short from_mask = 63;
	// To
	constexpr unsigned short to_mask = 63 << to_bits;
	// Special move
	constexpr unsigned short flag_mask = 3 << flag_bits;
	// Promotion piece
	constexpr unsigned short prom_mask = 3 << prom_bits;

	enum SpecialMove
	{
		NONE = 0,
		CAPTURE_ENPASSANT = 1,
		CASTLE = 2,
		PROMOTE = 3,
	};
};

#endif