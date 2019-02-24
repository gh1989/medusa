/*
Taken from Leela Chess
*/

#pragma once
#include <cstdint>
#ifdef _MSC_VER
#include <intrin.h>
#endif

namespace lczero {

inline unsigned long GetLowestBit(std::uint64_t value) {
#if defined(_MSC_VER) && defined(_WIN64)
    unsigned long result;
    _BitScanForward64(&result, value);
    return result;
#elif defined(_MSC_VER)
    unsigned long result;
    if (value & 0xFFFFFFFF) {
      _BitScanForward(&result, value);
    } else {
      _BitScanForward(&result, value >> 32);
      result += 32;
    }
    return result;
#else
    return __builtin_ctzll(value);
#endif
}

// Iterates over all set bits of the value, lower to upper. The value of
// dereferenced iterator is bit number (lower to upper, 0 bazed)
template <typename T>
class BitIterator {
 public:
  BitIterator(std::uint64_t value) : value_(value){};
  bool operator!=(const BitIterator& other) { return value_ != other.value_; }

  void operator++() { value_ &= (value_ - 1); }
  T operator*() const { return GetLowestBit(value_); }

 private:
  std::uint64_t value_;
};

class IterateBits {
 public:
  IterateBits(std::uint64_t value) : value_(value) {}
  BitIterator<int> begin() { return value_; }
  BitIterator<int> end() { return 0; }

 private:
  std::uint64_t value_;
};

}  // namespace lczero
