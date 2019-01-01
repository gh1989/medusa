#include "bitboard.h"

w_array<Bitboard, Bitboard::NUMBER_SQUARES> Bitboard::_knight_attacks;
w_array<Bitboard, Bitboard::NUMBER_SQUARES> Bitboard::_king_attacks;
w_array<Bitboard, Bitboard::NUMBER_SQUARES> Bitboard::_squares;
w_array<Bitboard, Bitboard::NUMBER_FILES> Bitboard::_files;
w_array<Bitboard, Bitboard::NUMBER_RANKS> Bitboard::_ranks;

w_array<Bitboard, Bitboard::NUMBER_SQUARES> Bitboard::_pawn_attacks;
w_array<Bitboard, Bitboard::NUMBER_SQUARES> Bitboard::_pawn_pushes;
w_array<Bitboard, Bitboard::NUMBER_SQUARES> Bitboard::_anti_diag_masks;
w_array<Bitboard, Bitboard::NUMBER_SQUARES> Bitboard::_diag_masks;
w_array<Bitboard, Bitboard::NUMBER_SQUARES> Bitboard::_rank_masks;
w_array<Bitboard, Bitboard::NUMBER_SQUARES> Bitboard::_file_masks;

w_array<Bitboard, Bitboard::NUMBER_SQUARES>  Bitboard::_attacks_S;
w_array<Bitboard, Bitboard::NUMBER_SQUARES>  Bitboard::_attacks_SE;
w_array<Bitboard, Bitboard::NUMBER_SQUARES>  Bitboard::_attacks_E;
w_array<Bitboard, Bitboard::NUMBER_SQUARES>  Bitboard::_attacks_NE;
w_array<Bitboard, Bitboard::NUMBER_SQUARES>  Bitboard::_attacks_N;
w_array<Bitboard, Bitboard::NUMBER_SQUARES>  Bitboard::_attacks_NW;
w_array<Bitboard, Bitboard::NUMBER_SQUARES>  Bitboard::_attacks_W;
w_array<Bitboard, Bitboard::NUMBER_SQUARES>  Bitboard::_attacks_SW;

w_array<int, 64> Bitboard::central_importance;

Bitboard Bitboard::_king_castle_path;
Bitboard Bitboard::_queen_castle_path;

void Bitboard::populate()
{
	const int central[NUMBER_SQUARES] = {
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  1,  1,  1,  1,  1,  1,  0,
		0,  1,  2,  2,  2,  2,  1,  0,
		0,  1,  2,  4,  4,  2,  1,  0,
		0,  1,  2,  4,  4,  2,  1,  0,
		0,  1,  2,  2,  2,  2,  1,  0,
		0,  1,  1,  1,  1,  1,  1,  0,
		0,  0,  0,  0,  0,  0,  0,  0
	};
		
	for (auto i = 0; i < NUMBER_SQUARES; ++i)
		central_importance.data[i] = central[i];

	// File bitboards
	_files.data[Bitboard::A_FILE] = Bitboard(0x0101010101010101);
	for (int i = Bitboard::B_FILE; i < Bitboard::NUMBER_FILES; ++i) {
		_files.data[i] = _files.data[0] << i;
	}

	// _ranks bitboards
	_ranks.data[Bitboard::RANK_1] = Bitboard(0xFF);
	for (int i = Bitboard::RANK_2; i <Bitboard::NUMBER_RANKS; ++i) {
		_ranks.data[i] = _ranks.data[0] << 8 * i;
	}

	// Square bitboards
	for (int i = Bitboard::RANK_1; i<Bitboard::NUMBER_RANKS; ++i)
	for (int j = Bitboard::A_FILE; j<Bitboard::NUMBER_FILES; ++j)
	{
		_squares.data[i * 8 + j] = _ranks.data[i] & _files.data[j];
	}

	//_______________________________________________________________________
	// Attacks north-west.
	//_______________________________________________________________________
	for (int i = Bitboard::a1; i <= Bitboard::h8; ++i)
	{
		Bitboard sqr = _squares.data[i];
		if (!(sqr & _files.data[Bitboard::A_FILE]).empty() || !(sqr & _ranks.data[Bitboard::RANK_8]).empty())
		{
			_attacks_NW.data[i] = Bitboard(0);
			continue;
		}

		auto attack = sqr << 7;
		int ii = 2;
		while ((attack&_files.data[Bitboard::A_FILE]).empty() && (attack&_ranks.data[Bitboard::RANK_8]).empty())
		{
			attack = attack | (sqr << (7 * ii++));
		}
		_attacks_NW.data[i] = attack;
	}
	//_______________________________________________________________________
	// Attacks north-east.
	//_______________________________________________________________________
	for (int i = Bitboard::a1; i <= Bitboard::h8; ++i)
	{
		Bitboard sqr = _squares.data[i];
		if (!(sqr & _files.data[Bitboard::H_FILE]).empty() || !(sqr & _ranks.data[Bitboard::RANK_8]).empty())
		{
			_attacks_NE.data[i] = Bitboard(0);
			continue;
		}

		auto attack = sqr << 9;
		int ii = 2;
		while ((attack&_files.data[Bitboard::H_FILE]).empty() && (attack&_ranks.data[Bitboard::RANK_8]).empty())
		{
			attack = attack | (sqr << (9 * ii++));
		}
		_attacks_NE.data[i] = attack;
	}
	//_______________________________________________________________________
	// Attacks south-east.
	//_______________________________________________________________________
	for (int i = Bitboard::a1; i <= Bitboard::h8; ++i)
	{
		Bitboard sqr = _squares.data[i];
		if (!(sqr & _files.data[Bitboard::H_FILE]).empty() || !(sqr & _ranks.data[Bitboard::RANK_1]).empty())
		{
			_attacks_SE.data[i] = Bitboard(0);
			continue;
		}

		auto attack = sqr >> 7;
		int ii = 2;
		while ((attack&_files.data[Bitboard::H_FILE]).empty() && (attack&_ranks.data[Bitboard::RANK_1]).empty())
		{
			attack = attack | (sqr >> (7 * ii++));
		}
		_attacks_SE.data[i] = attack;
	}
	//_______________________________________________________________________
	// Attacks south-west.
	//_______________________________________________________________________
	for (int i = Bitboard::a1; i <= Bitboard::h8; ++i)
	{
		Bitboard sqr = _squares.data[i];
		if (!(sqr & _files.data[Bitboard::A_FILE]).empty() || !(sqr & _ranks.data[Bitboard::RANK_1]).empty())
		{
			_attacks_SW.data[i] = Bitboard(0);
			continue;
		}

		auto attack = sqr >> 9;
		int ii = 2;
		while ((attack&_files.data[Bitboard::A_FILE]).empty() && (attack&_ranks.data[Bitboard::RANK_1]).empty())
		{
			attack = attack | (sqr >> (9 * ii++));
		}
		_attacks_SW.data[i] = attack;
	}
	//_______________________________________________________________________
	// Attacks west.
	//_______________________________________________________________________
	for (int i = Bitboard::a1; i <= Bitboard::h8; ++i)
	{
		Bitboard sqr = _squares.data[i];
		if (!(sqr & _files.data[Bitboard::A_FILE]).empty())
		{
			_attacks_W.data[i] = Bitboard(0);
			continue;
		}

		auto attack = sqr >> 1;
		int ii = 2;
		while ((attack&_files.data[Bitboard::A_FILE]).empty())
		{
			attack = attack | (sqr >> (ii++));
		}
		_attacks_W.data[i] = attack;
	}
	//_______________________________________________________________________
	// Attacks east.
	//_______________________________________________________________________
	for (int i = Bitboard::a1; i <= Bitboard::h8; ++i)
	{
		Bitboard sqr = _squares.data[i];
		if (!(sqr & _files.data[Bitboard::H_FILE]).empty())
		{
			_attacks_E.data[i] = Bitboard(0);
			continue;
		}

		auto attack = sqr << 1;
		int ii = 2;
		while ((attack&_files.data[Bitboard::H_FILE]).empty())
		{
			attack = attack | (sqr << (ii++));
		}
		_attacks_E.data[i] = attack;
	}
	//_______________________________________________________________________
	// Attacks south.
	//_______________________________________________________________________
	for (int i = Bitboard::a1; i < Bitboard::h8; ++i)
	{
		Bitboard sqr = _squares.data[i];
		if (!(sqr & _ranks.data[Bitboard::RANK_1]).empty())
		{
			_attacks_S.data[i] = Bitboard(0);
			continue;
		}

		auto attack = sqr >> 8;
		int ii = 2;
		while ((attack&_ranks.data[Bitboard::RANK_1]).empty())
		{
			attack = attack | (sqr >> (8 * ii++));
		}
		_attacks_S.data[i] = attack;
	}
	//_______________________________________________________________________
	// Attacks north.
	//_______________________________________________________________________
	for (int i = Bitboard::a1; i <= Bitboard::h8; ++i)
	{
		Bitboard sqr = _squares.data[i];
		if (!(sqr & _ranks.data[Bitboard::RANK_8]).empty())
		{
			_attacks_N.data[i] = Bitboard(0);
			continue;
		}

		auto attack = sqr << 8;
		int ii = 2;
		while ((attack&_ranks.data[Bitboard::RANK_8]).empty())
		{
			attack = attack | (sqr << (8 * ii++));
		}
		_attacks_N.data[i] = attack;
	}

	//_______________________________________________________________________
	// Knight attacks
	//_______________________________________________________________________
	for (int i = Bitboard::a1; i <= Bitboard::h8; ++i)
	{
		Bitboard s = _squares.data[i];
		Bitboard attack(0);
		if ((s&(_ranks.data[Bitboard::RANK_1] | _ranks.data[Bitboard::RANK_2] | _files.data[Bitboard::A_FILE])).empty())
			attack = attack | (s >> 17);
		if ((s&(_ranks.data[Bitboard::RANK_1] | _ranks.data[Bitboard::RANK_2] | _files.data[Bitboard::H_FILE])).empty())
			attack = attack | s >> 15;
		if ((s&(_ranks.data[Bitboard::RANK_1] | _files.data[Bitboard::A_FILE] | _files.data[Bitboard::B_FILE])).empty())
			attack = attack | s >> 10;
		if ((s&(_ranks.data[Bitboard::RANK_1] | _files.data[Bitboard::H_FILE] | _files.data[Bitboard::G_FILE])).empty())
			attack = attack | s >> 6;
		if ((s&(_ranks.data[Bitboard::RANK_8] | _files.data[Bitboard::A_FILE] | _files.data[Bitboard::B_FILE])).empty())
			attack = attack | s << 6;
		if ((s&(_ranks.data[Bitboard::RANK_8] | _files.data[Bitboard::G_FILE] | _files.data[Bitboard::H_FILE])).empty())
			attack = attack | s << 10;
		if ((s&(_ranks.data[Bitboard::RANK_8] | _ranks.data[Bitboard::RANK_7] | _files.data[Bitboard::A_FILE])).empty())
			attack = attack | s << 15;
		if ((s&(_ranks.data[Bitboard::RANK_8] | _ranks.data[Bitboard::RANK_7] | _files.data[Bitboard::H_FILE])).empty())
			attack = attack | s << 17;
		_knight_attacks.data[i] = attack;
	}
	//_______________________________________________________________________
	// King attacks
	//_______________________________________________________________________
	for (int i = Bitboard::a1; i <= Bitboard::h8; ++i)
	{
		Bitboard s = _squares.data[i];
		Bitboard attack(0);
		if ((s&(_ranks.data[Bitboard::RANK_8] | _files.data[Bitboard::A_FILE])).empty())
			attack = attack | (s << 7);
		if ((s&(_ranks.data[Bitboard::RANK_1] | _files.data[Bitboard::A_FILE])).empty())
			attack = attack | (s >> 9);
		if ((s&(_ranks.data[Bitboard::RANK_8] | _files.data[Bitboard::H_FILE])).empty())
			attack = attack | (s << 9);
		if ((s&(_ranks.data[Bitboard::RANK_1] | _files.data[Bitboard::H_FILE])).empty())
			attack = attack | (s >> 7);
		if ((s&_files.data[Bitboard::A_FILE]).empty())
			attack = attack | (s >> 1);
		if ((s&_files.data[Bitboard::H_FILE]).empty())
			attack = attack | (s << 1);
		if ((s&_ranks.data[Bitboard::RANK_8]).empty())
			attack = attack | (s << 8);
		if ((s&_ranks.data[Bitboard::RANK_1]).empty())
			attack = attack | (s >> 8);
		_king_attacks.data[i] = attack;
	}

	//_______________________________________________________________________
	// (White) Pawn attacks
	//_______________________________________________________________________
	for (int i = Bitboard::a1; i <= Bitboard::h8; ++i)
	{
		Bitboard s = _squares.data[i];
		Bitboard attack(0);
		if ((s&_ranks.data[Bitboard::RANK_8]).empty())
		{
			if ((s&_files.data[Bitboard::A_FILE]).empty())
			{
				attack = attack | (s << 7);
			}

			if ((s&_files.data[Bitboard::H_FILE]).empty())
			{
				attack = attack | (s << 9);
			}
			_pawn_attacks.data[i] = attack;
		}
	}

	//_______________________________________________________________________
	// (White) Pawn pushess
	//_______________________________________________________________________
	for (int i = Bitboard::a1; i <= Bitboard::h8; ++i)
	{
		Bitboard s = _squares.data[i];
		Bitboard attack(0);
		if ((s&_ranks.data[Bitboard::RANK_8]).empty())
		{
			attack = attack | (s << 8);
			if (!(s&_ranks.data[Bitboard::RANK_2]).empty())
			{
				attack = attack | (s << 16);
			}
			_pawn_pushes.data[i] = attack;
		}
	}

	//_______________________________________________________________________
	// Diagonals, anti-diagonals, rank and file masks for a square
	//_______________________________________________________________________
	for (int i = Bitboard::a1; i <= Bitboard::h8; ++i)
	{
		Bitboard s = _squares.data[i];

		// Diagonal attacks
		_diag_masks.data[i] = _attacks_NE.data[i] | _attacks_SW.data[i] | s;

		// Anti - diagonal attacks
		_anti_diag_masks.data[i] = _attacks_NW.data[i] | _attacks_SE.data[i] | s;

		// Rank masks
		_rank_masks.data[i] = _ranks.data[int(i/8)];

		// File masks
		_file_masks.data[i] = _files.data[i % 8];
	}

	// Castling path.
	_king_castle_path = _squares.data[Bitboard::f1] | _squares.data[Bitboard::g1];
	_queen_castle_path = _squares.data[Bitboard::d1] | _squares.data[Bitboard::c1];
}
	
uint64_t Bitboard::rotate180(uint64_t x)
{
	uint64_t h1 = 0x5555555555555555;
	uint64_t h2 = 0x3333333333333333;
	uint64_t h4 = 0x0F0F0F0F0F0F0F0F;
	uint64_t v1 = 0x00FF00FF00FF00FF;
	uint64_t v2 = 0x0000FFFF0000FFFF;
	x = ((x >> 1) & h1) | ((x & h1) << 1);
	x = ((x >> 2) & h2) | ((x & h2) << 2);
	x = ((x >> 4) & h4) | ((x & h4) << 4);
	x = ((x >> 8) & v1) | ((x & v1) << 8);
	x = ((x >> 16) & v2) | ((x & v2) << 16);
	x = (x >> 32) | (x << 32);
	return x & 0xFFFFFFFFFFFFFFFF;
}

Bitboard Bitboard::slider_attacks(
	const Bitboard& occupancy,
	Square square,
	const Bitboard& direction_mask)
{
	uint64_t potential_blockers = (occupancy & direction_mask).bit_number;
	uint64_t diff = potential_blockers - 2 * _squares.data[square].bit_number;
	uint64_t changed = diff ^ occupancy.bit_number;
	return changed & direction_mask.bit_number;
}

Bitboard Bitboard::diag_attacks(
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

Bitboard Bitboard::antidiag_attacks(
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

Bitboard Bitboard::file_attacks(
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

Bitboard Bitboard::rank_attacks(
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

Bitboard Bitboard::rook_attacks(const Bitboard& occupancy, Square square)
{
	uint64_t occupancy_int = occupancy.get_bit_number();
	return rank_attacks(occupancy_int, square) | file_attacks(occupancy_int, square);
}

Bitboard Bitboard::bishop_attacks(const Bitboard& occupancy, Square square)
{
	uint64_t occupancy_int = occupancy.get_bit_number();
	return diag_attacks(occupancy_int, square) | antidiag_attacks(occupancy_int, square);
}

Bitboard Bitboard::queen_attacks(const Bitboard& occupancy, Square square)
{
	return bishop_attacks(occupancy, square) | rook_attacks(occupancy, square);
}

bool Bitboard::is_square_attacked(
	const Bitboard& occupancy,
	const Bitboard& square,
	const Bitboard& pawns,
	const Bitboard& bishops,
	const Bitboard& knights,
	const Bitboard& queens,
	const Bitboard& rooks,
	const Bitboard& kings,
	bool reverse_pawn_attacks)
{
	// __knights__
	auto s = square.bit_length();
	auto attacks = _knight_attacks.data[s].bit_number;
	if (attacks & knights.bit_number)
		return true;

	// __kings__
	attacks = _king_attacks.data[s].bit_number;
	if (attacks & kings.bit_number)
		return true;

	// __bishops__
	attacks = bishop_attacks(occupancy, s).bit_number;
	if (attacks & bishops.bit_number)
		return true;

	// __rooks__
	attacks = rook_attacks(occupancy, s).bit_number;
	if (attacks & rooks.bit_number)
		return true;

	// __queens__
	attacks = queen_attacks(occupancy, s).bit_number;
	if (attacks & queens.bit_number)
		return true;

	// __pawns__
	auto pawn_square = square.bit_number; // potentially attacked square.
	if (!reverse_pawn_attacks)
		pawn_square = rotate180(pawn_square);
	auto new_pawn_square = Bitboard(pawn_square);
	auto pawn_s = new_pawn_square.bit_length();
	auto pawn_bits = pawns.bit_number;
	attacks = _pawn_attacks.data[pawn_s].bit_number;
	if (!reverse_pawn_attacks)
		attacks = rotate180(attacks);
	if (attacks & pawn_bits)
		return true;

	return false;
}

w_array<int, Bitboard::NUMBER_SQUARES> Bitboard::attacks_pressure_matrix(
	const Bitboard& occupancy,
	const Bitboard& pawns,
	const Bitboard& bishops,
	const Bitboard& knights,
	const Bitboard& queens,
	const Bitboard& rooks,
	const Bitboard& kings,
	bool reverse_pawn_attacks)
{
	w_array<int, Bitboard::NUMBER_SQUARES> ret;
	for (int i = 0; i < Bitboard::NUMBER_SQUARES; i++)
		ret.data[i] = 0;

	// This is suspicious
	for (auto square : pawns.non_empty_squares())
	{
		// this pawn square
		auto pawn_square = _squares.data[square];  
		auto pawn_square_number = pawn_square.bit_number;
		// rotate 180 the black pawn square
		if (reverse_pawn_attacks)
			pawn_square_number = rotate180(pawn_square_number);
		// create the new bitboard
		auto new_pawn_square = Bitboard(pawn_square_number);
		// so find the real index
		auto pawn_s = new_pawn_square.bit_length();
		// get the (potentially rotated attacks)
		auto attacks_number = _pawn_attacks.data[pawn_s].bit_number;
		// rotate them back
		if (reverse_pawn_attacks)
			attacks_number = rotate180(attacks_number);
		auto attacks = Bitboard(attacks_number);
		ret = ret + attacks.to_pressure_matrix(9);
	}

	for (auto square : knights.non_empty_squares())
	{
		auto attacks = _knight_attacks.data[square];
		ret = ret + attacks.to_pressure_matrix(3);
	}

	for (auto square : bishops.non_empty_squares())
	{
		auto attacks = bishop_attacks(occupancy, square);
		ret = ret + attacks.to_pressure_matrix(3);
	}

	for (auto square : queens.non_empty_squares())
	{
		auto attacks = queen_attacks(occupancy, square);
		ret = ret + attacks.to_pressure_matrix(1);
	}

	for (auto square : rooks.non_empty_squares())
	{
		auto attacks = rook_attacks(occupancy, square);
		ret = ret + attacks.to_pressure_matrix(2);
	}

	/* This makes the king walk around the board...
	for (auto square : kings.non_empty_squares())
	{
		auto attacks = _king_attacks.data[square];
		ret = ret + attacks.to_pressure_matrix();
	}
	*/
	return ret;
}

w_array<int, Bitboard::NUMBER_SQUARES> Bitboard::to_pressure_matrix(int value) const
{
	w_array<int, Bitboard::NUMBER_SQUARES> pressure_matrix;
	uint64_t bit = 1;
	for (int square = Bitboard::a1; square < Bitboard::NUMBER_SQUARES; square++)
		pressure_matrix.data[square] = (bit_number & (bit << square)) ? value : 0;
	return pressure_matrix;
}
