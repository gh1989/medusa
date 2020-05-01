#ifndef bitboard_h
#define bitboard_h

#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <intrin.h>

#include "types.h"

namespace Medusa
{
	template<class T>
	class BitIterator {
	public:
		BitIterator(uint64_t value) : value_(value) {};
		bool operator!=(const BitIterator& other) {
			return value_ != other.value_;
		}
		void operator++() { value_ &= (value_ - 1); }
		unsigned int operator*() const { return value_.nLSB(); }

	private:
		T value_;
	};
		
	class Bitboard
	{
	public:
		Bitboard() : bit_number(0) {}
		Bitboard(uint64_t bit_number_) : bit_number(bit_number_) {}
		
		bool operator==(const Bitboard& other) const
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

		Bitboard operator- (int value) const
		{
			return bit_number - value;
		}
	
		void operator &= (const Bitboard& other)
		{
			bit_number &= other.bit_number;
		}

		operator bool() const
		{
			return bit_number != 0;
		}
		// population count				
		int PopCnt() const
		{
			return __popcnt64(bit_number);
		}

		// returns the position of the most significant bit
		unsigned int nMSB() const
		{
			uint64_t msb_val = msb64(bit_number);
			unsigned int msb_num = 0;
			while (msb_val > 1)
			{
				msb_num++;
				msb_val >>= 1;
			}
			return msb_num;
		}

		unsigned int nLSB() const
		{
			unsigned long result;
			_BitScanForward64(&result, bit_number);
			return result;
		}

		BitIterator<Bitboard> begin()
		{
			return bit_number; 
		}
		
		BitIterator<Bitboard> end()
		{
			return 0;
		}

	private:

		uint64_t msb64(register uint64_t x) const
		{
			x |= (x >> 1);
			x |= (x >> 2);
			x |= (x >> 4);
			x |= (x >> 8);
			x |= (x >> 16);
			x |= (x >> 32);
			return(x & ~(x >> 1));
		}

		uint64_t bit_number;
	};

	Bitboard Rotate180(Bitboard bb);
	uint64_t _ByteSwap(uint64_t to_swap);
};

#endif
