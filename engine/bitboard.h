#ifndef bitboard_h
#define bitboard_h

#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <intrin.h>

#include "types.h"

class Bitboard
{
public:
	
	enum Rank
	{
		RANK_1 = 0,
		RANK_2,
		RANK_3,
		RANK_4,
		RANK_5,
		RANK_6,
		RANK_7,
		RANK_8,
		NUMBER_RANKS
	};
	
	enum File
	{
		A_FILE = 0,
		B_FILE,
		C_FILE,
		D_FILE,
		E_FILE,
		F_FILE,
		G_FILE,
		H_FILE,
		NUMBER_FILES
	};


	enum Square
	{
		NOTSET = -1,
		a1=0, b1, c1, d1, e1, f1, g1, h1,
		a2, b2, c2, d2, e2, f2, g2, h2,
		a3, b3, c3, d3, e3, f3, g3, h3,
		a4, b4, c4, d4, e4, f4, g4, h4,
		a5, b5, c5, d5, e5, f5, g5, h5,
		a6, b6, c6, d6, e6, f6, g6, h6,
		a7, b7, c7, d7, e7, f7, g7, h7,
		a8, b8, c8, d8, e8, f8, g8, h8,
		NUMBER_SQUARES
	};

	
	Bitboard() : bit_number(0) {}
	Bitboard(uint64_t bit_number_) : bit_number(bit_number_) {}

	static uint64_t byteswap(uint64_t to_swap) {
#ifdef __GNUC__
		return __builtin_bswap64(to_swap);
#elif _MSC_VER
		return _byteswap_uint64(to_swap);
#endif
	}
	static void populate();
	uint64_t get_bit_number() const { return bit_number; }

	bool operator==(const Bitboard& other)
	{
		return bit_number == other.bit_number;
	}

	Bitboard operator<<(int x) const
	{
		return Bitboard(bit_number << x);
	}

	Bitboard operator~() const
	{
		return Bitboard(~bit_number);
	}

	Bitboard operator>>(int x) const
	{
		return Bitboard(bit_number >> x);
	}

	Bitboard operator| (const Bitboard& other) const
	{
		return Bitboard(bit_number | other.bit_number);
	}

    Bitboard operator& (const Bitboard& other) const
	{
		return Bitboard(bit_number & other.bit_number);
	}

	Bitboard operator^ (const Bitboard& other) const
	{
		return Bitboard(bit_number ^ other.bit_number);
	}

	Bitboard __xor__(const Bitboard &other) const
	{
		return Bitboard(bit_number ^ other.bit_number);
	}

	bool __eq__(const Bitboard &other) const
	{
		return bit_number == other.bit_number;
	}

	bool __bool__(const Bitboard &other) const
	{
		return bit_number == 0ULL;
	}
	
	uint64_t __hash__() const
	{
		return bit_number;
	}

	Square bit_length() const
	{
		uint64_t bits = bit_number;
		int x;
		for (x = 0; bits != 0; x++)
		{
			bits >>= 1;
		}
		
		return static_cast<Square>(x-1);
	}

	int on_bits() const
	{
		return __popcnt64(bit_number);
	}

	bool empty() const
	{
		return bit_number == 0;
	}

	bool any() const
	{
		return !empty();
	}

	Bitboard reflect() const
	{
		Bitboard b = *this;
		return	((b & _ranks.data[RANK_1]) << 56) |
				((b & _ranks.data[RANK_2]) << 40) |
				((b & _ranks.data[RANK_3]) << 24) |
				((b & _ranks.data[RANK_4]) << 8)  |
				((b & _ranks.data[RANK_5]) >> 8)  |
				((b & _ranks.data[RANK_6]) >> 24) |
				((b & _ranks.data[RANK_7]) >> 40) |
				((b & _ranks.data[RANK_8]) >> 56);
	}

	static Square reflect(Square square)
	{
		int square_int = static_cast<int>(square);
		square_int = (square_int % 8) + (7 - (square_int / 8))*8;
		Square output = static_cast<Square>(square_int);
		return output;
	}
	
	Bitboard off_on_bit(Square off, Square on) const
	{
		return off_bit(off).on_bit(on);
	}

	bool is_on(Square square) const
	{
		Bitboard b = *this;
		Bitboard s = _squares.data[square];
		return bool((b & s).bit_number);
	}

	Bitboard off_bit(Square off) const
	{
		Bitboard b = *this;
		Bitboard s = _squares.data[off];
		return b & (~s);
	}

	Bitboard on_bit(Square on) const
	{
		Bitboard b = *this;
		Bitboard s = _squares.data[on];
		return b | s;
	}

	static uint64_t rotate180(uint64_t x);

	static std::string square_name(Bitboard::Square square)
	{
		const std::string file_strings[8] = { "a", "b", "c", "d", "e", "f", "g", "h"}; 
		const std::string rank_strings[8] = { "1", "2", "3", "4", "5", "6", "7", "8"};
		int square_int = static_cast<int>(square);
		return file_strings[square_int%8] + rank_strings[int(square_int/8)];
	}
	
	static w_array<Bitboard, NUMBER_SQUARES> get_knight_attacks() { return _knight_attacks; }
	static w_array<Bitboard, NUMBER_SQUARES> get_king_attacks() { return _king_attacks; }
	static w_array<Bitboard, NUMBER_SQUARES> get_pawn_attacks() { return _pawn_attacks; }
	static w_array<Bitboard, NUMBER_SQUARES> get_pawn_pushes() { return _pawn_pushes; }

	// Get diag masks
	static w_array<Bitboard, NUMBER_SQUARES> get_diag_masks() { return _diag_masks;  }
	static Bitboard get_diag_mask(Square square) {
		return _diag_masks.data[square];
	}

	// Get anti diag masks
	static w_array<Bitboard, NUMBER_SQUARES> get_anti_diag_masks() { return _anti_diag_masks; }

	static Bitboard get_anti_diag_mask(Square square) {
		return _anti_diag_masks.data[square];
	}

	// Get rank masks
	static w_array<Bitboard, NUMBER_SQUARES> get_rank_masks() { return _rank_masks; }
	static Bitboard get_rank_mask(Square square) 
	{
		return _rank_masks.data[square];
	}

	// Get file masks
	static w_array<Bitboard, NUMBER_SQUARES> get_file_masks() { return _file_masks; }
	static Bitboard get_file_mask(Square square) {
		return _file_masks.data[square];
	}
	
	std::vector<Bitboard> non_empty_square_bitboards() const
	{
		std::vector<Bitboard> ret(on_bits());
		int k = 0;
		for(int s = a1; s< NUMBER_SQUARES; s++)
		{
			auto square = _squares.data[s];
			if (square.bit_number & bit_number)
			{
				ret[k++] = square;
			}
		}

		return ret;
	}

	#ifndef SWIG
	std::vector<Square> non_empty_squares() const
	{
		std::vector<Square> ret(on_bits());
		int k = 0;
		for(int s = a1; s<NUMBER_SQUARES; s++)
		{
			auto square = _squares.data[s];
			if (square.bit_number & bit_number)
			{
				ret[k++] = static_cast<Square>(s);
			}
		}
		return ret;
	}
	#endif
	
	// Get files, ranks, squares
	static w_array<Bitboard, NUMBER_FILES> get_files() { return _files; }
	static w_array<Bitboard, NUMBER_RANKS> get_ranks() { return _ranks; }
	static w_array<Bitboard, NUMBER_SQUARES> get_squares() { return _squares; }

	// Get king/queenside castling paths
	static Bitboard get_king_castle_path() { return _king_castle_path; }
	static Bitboard get_queen_castle_path() { return _queen_castle_path;  }

	static w_array<Bitboard, NUMBER_SQUARES> _knight_attacks;
	static w_array<Bitboard, NUMBER_SQUARES> _king_attacks;
	static w_array<Bitboard, NUMBER_SQUARES> _pawn_attacks;
	static w_array<Bitboard, NUMBER_SQUARES> _pawn_pushes;

	static w_array<Bitboard, NUMBER_SQUARES> _squares;
	static w_array<Bitboard, NUMBER_FILES> _files;
	static w_array<Bitboard, NUMBER_RANKS> _ranks;

	static w_array<Bitboard, NUMBER_SQUARES> _anti_diag_masks;
	static w_array<Bitboard, NUMBER_SQUARES> _diag_masks;
	static w_array<Bitboard, NUMBER_SQUARES> _rank_masks;
	static w_array<Bitboard, NUMBER_SQUARES> _file_masks;


	// Attack Bitboards
	static w_array<Bitboard, NUMBER_SQUARES> get_attacks_S() { return _attacks_S; }
	static w_array<Bitboard, NUMBER_SQUARES> get_attacks_SE() { return _attacks_SE; }
	static w_array<Bitboard, NUMBER_SQUARES> get_attacks_E() { return _attacks_E; }
	static w_array<Bitboard, NUMBER_SQUARES> get_attacks_NE() { return _attacks_NE; }
	static w_array<Bitboard, NUMBER_SQUARES> get_attacks_N() { return _attacks_N; }
	static w_array<Bitboard, NUMBER_SQUARES> get_attacks_NW() { return _attacks_NW; }
	static w_array<Bitboard, NUMBER_SQUARES> get_attacks_W() { return _attacks_W; }
	static w_array<Bitboard, NUMBER_SQUARES> get_attacks_SW() { return _attacks_SW; }

	static w_array<Bitboard, NUMBER_SQUARES>  _attacks_S;
	static w_array<Bitboard, NUMBER_SQUARES>  _attacks_SE;
	static w_array<Bitboard, NUMBER_SQUARES>  _attacks_E;
	static w_array<Bitboard, NUMBER_SQUARES>  _attacks_NE;
	static w_array<Bitboard, NUMBER_SQUARES>  _attacks_N;
	static w_array<Bitboard, NUMBER_SQUARES>  _attacks_NW;
	static w_array<Bitboard, NUMBER_SQUARES>  _attacks_W;
	static w_array<Bitboard, NUMBER_SQUARES>  _attacks_SW;

	static Bitboard _king_castle_path;
	static Bitboard _queen_castle_path;

	static Bitboard slider_attacks(
		const Bitboard& occupancy,
		Square square,
		const Bitboard& direction_mask)
	{
		uint64_t potential_blockers = (occupancy & direction_mask).bit_number;
		uint64_t diff = potential_blockers - 2 * _squares.data[square].bit_number;
		uint64_t changed = diff ^ occupancy.bit_number;
		return changed & direction_mask.bit_number;
	}

	static Bitboard diag_attacks(
		const Bitboard& occupancy,
		Square square)
	{
		auto diag_mask = get_diag_mask(square);
		auto forward_part = slider_attacks(occupancy, square, diag_mask);
		auto sqr_bit_num = _squares.data[square].bit_number;
		Bitboard reversed_square_bitboard = byteswap(sqr_bit_num);
		uint64_t reversed_occupancy = byteswap(occupancy.bit_number);
		Square reversed_square = reversed_square_bitboard.bit_length();
		auto anti_diag_mask_for_reversed = get_anti_diag_mask(reversed_square);
		auto reversed_part = slider_attacks(reversed_occupancy, reversed_square, anti_diag_mask_for_reversed);
		return forward_part.bit_number | byteswap(reversed_part.bit_number);
	}

	static Bitboard antidiag_attacks(
		const Bitboard& occupancy,
		Square square)
	{
		auto anti_diag_mask = get_anti_diag_mask(square);
		auto forward_part = slider_attacks(occupancy, square, anti_diag_mask);
		auto sqr_bit_num = _squares.data[square].bit_number;
		Bitboard reversed_square_bitboard = byteswap(sqr_bit_num);
		uint64_t reversed_occupancy = byteswap(occupancy.bit_number);
		Square reversed_square = reversed_square_bitboard.bit_length();
		auto diag_mask_for_reversed = get_diag_mask(reversed_square);
		auto reversed_part = slider_attacks(reversed_occupancy, reversed_square, diag_mask_for_reversed);
		return forward_part.bit_number | byteswap(reversed_part.bit_number);
	}

	static Bitboard file_attacks(
		const Bitboard& occupancy,
		Square square)
	{
		auto file_mask = get_file_mask(square);
		auto forward_part = slider_attacks(occupancy, square, file_mask);
		auto sqr_bit_num = _squares.data[square].bit_number;
		Bitboard reversed_square_bitboard = byteswap(sqr_bit_num);
		uint64_t reversed_occupancy = byteswap(occupancy.bit_number);
		Square reversed_square = reversed_square_bitboard.bit_length();
		auto file_mask_for_reversed = get_file_mask(reversed_square);
		auto reversed_part = slider_attacks(reversed_occupancy, reversed_square, file_mask_for_reversed);
		return forward_part.bit_number | byteswap(reversed_part.bit_number);
	}

	static Bitboard rank_attacks(
		const Bitboard& occupancy,
		Square square)
	{
		auto rank_mask = get_rank_mask(square);
		auto forward_part = slider_attacks(occupancy, square, rank_mask);
		auto sqr_bit_num = _squares.data[square].bit_number;
		Bitboard rotated_square_bitboard = rotate180(sqr_bit_num);
		uint64_t rotated_occupancy = rotate180(occupancy.bit_number);
		Square rotated_square = rotated_square_bitboard.bit_length();
		auto rank_mask_for_rotated = get_rank_mask(rotated_square);
		auto rotated_part = slider_attacks(rotated_occupancy, rotated_square, rank_mask_for_rotated);
		return forward_part.bit_number | rotate180(rotated_part.bit_number);
	}
	
	static Bitboard rook_attacks(const Bitboard& occupancy, Square square)
	{
		uint64_t occupancy_int = occupancy.get_bit_number();
		return rank_attacks(occupancy_int, square) | file_attacks(occupancy_int, square);
	}

	static Bitboard bishop_attacks(const Bitboard& occupancy, Square square)
	{
		uint64_t occupancy_int = occupancy.get_bit_number();
		return diag_attacks(occupancy_int, square) | antidiag_attacks(occupancy_int, square);
	}

	static Bitboard queen_attacks(const Bitboard& occupancy, Square square)
	{
		return bishop_attacks(occupancy, square) | rook_attacks(occupancy, square);
	}

	static bool is_square_attacked(
		const Bitboard& occupancy,
		const Bitboard& square,
		const Bitboard& pawns,
		const Bitboard& bishops,
		const Bitboard& knights,
		const Bitboard& queens,
		const Bitboard& rooks,
		const Bitboard& kings,
		bool reverse_pawn_attacks=false);

	static bool in_rank(Square square, Rank rank)
	{
		Square lower = static_cast<Square>(rank*8);
		Square higher = static_cast<Square>((rank+1)*8);
		return (square >= lower) && (square < higher);
	}

	static bool in_file(Square square, File file)
	{
		return static_cast<File>(square % 8) == file;
	}

private:
	uint64_t bit_number;
	
};

#endif
