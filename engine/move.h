#ifndef move_h
#define move_h

namespace Medusa
{
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
}

#endif
