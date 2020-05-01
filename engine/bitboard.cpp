#include "bitboard.h"

namespace Medusa
{
	Bitboard Rotate180(Bitboard bb)
	{
		const Bitboard h1 = 0x5555555555555555;
		const Bitboard h2 = 0x3333333333333333;
		const Bitboard h4 = 0x0F0F0F0F0F0F0F0F;
		const Bitboard v1 = 0x00FF00FF00FF00FF;
		const Bitboard v2 = 0x0000FFFF0000FFFF;
		const Bitboard m1 = (0xFFFFFFFFFFFFFFFF);

		bb = ((bb >> 1) & h1) | ((bb & h1) << 1);
		bb = ((bb >> 2) & h2) | ((bb & h2) << 2);
		bb = ((bb >> 4) & h4) | ((bb & h4) << 4);
		bb = ((bb >> 8) & v1) | ((bb & v1) << 8);
		bb = ((bb >> 16) & v2) | ((bb & v2) << 16);
		bb = (bb >> 32) | (bb << 32);
		return bb & m1;
	}

	uint64_t _ByteSwap(uint64_t to_swap)
	{
		return _byteswap_uint64(to_swap);
	}
}