#ifndef benchmark_h
#define benchmark_h

#include "utils.h"
#include "position.h"

namespace medusa
{
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

	void test_moves()
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

