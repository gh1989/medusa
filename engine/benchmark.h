#ifndef benchmark_h
#define benchmark_h

#include "utils.h"
#include "position.h"

namespace medusa
{
	void test_promotion_capture()
	{
		auto pos = position_from_fen("6r1/7P/p7/Pr6/4pPK1/2k1B3/5P2/8 w - - 0 52");
		auto legals = pos.legal_pawn_moves(1);
		for (auto m : legals)
		{
			std::cout << as_uci(m) << std::endl;
		}
	}

	void test_equality_operator()
	{
		std::array<Bitboard, 2> a{ Bitboard(0), Bitboard(1) };
		std::array<Bitboard, 2> b{ Bitboard(0), Bitboard(2) };
		if(a==b)
			std::cout << "(a == b)" << std::endl;
		else
			std::cout << "(a != b)" << std::endl;
	}

	void test_moves_sanity()
	{
		Position new_pos = position_from_fen("");
		new_pos.apply_uci("b1a3");
		new_pos.apply_uci("b8c6");
		new_pos.apply_uci("a3c4");
		new_pos.apply_uci("d7d5");
		new_pos.apply_uci("d2d4");
		new_pos.apply_uci("c6d4");

	}

	void benchmarks()
	{
		auto new_pos = position_from_fen("");
		new_pos.apply_uci("d2d4");
		new_pos.apply_uci("b8c6");
		new_pos.apply_uci("b1a3");
		auto moves = new_pos.legal_moves();
		for (auto move : moves)
		{
			std::cout << as_uci(move) << std::endl;
		}
		new_pos.pp();
	}
}

#endif

