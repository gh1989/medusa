#ifndef utils_h
#define utils_h

#include "position.h"

#include <iostream>
#include <memory>
#include <string>
#include <chrono>

namespace medusa
{
	// Piece string
	std::string piece_string(Piece piece, Colour c);

	// Piece strings lower
	std::string piece_string_lower(Piece piece);

	// Position from string (fen)
	Position position_from_fen(std::string str);

	// Bitboard from string
	Bitboard bitboard_from_string(std::string str);

	// Steady clock to system clock
	std::chrono::time_point<std::chrono::system_clock> SteadyClockToSystemClock(
		std::chrono::time_point<std::chrono::steady_clock> time);

	// Format time
	std::string FormatTime(std::chrono::time_point<std::chrono::system_clock> time);

	// Joins strings using @delim as delimiter.
	std::string StrJoin(const std::vector<std::string>& strings,
		const std::string& delim = " ");

	// Splits strings at whitespace.
	std::vector<std::string> StrSplitAtWhitespace(const std::string& str);

	// Split string by delimiter.
	std::vector<std::string> StrSplit(const std::string& str,
		const std::string& delim);

	// Parses comma-separated list of integers.
	std::vector<int> ParseIntList(const std::string& str);

	// Trims a string of whitespace from the start.
	std::string LeftTrim(std::string str);

	// Trims a string of whitespace from the end.
	std::string RightTrim(std::string str);

	// Trims a string of whitespace from both ends.
	std::string Trim(std::string str);

	// Returns whether strings are equal, ignoring case.
	bool StringsEqualIgnoreCase(const std::string& a, const std::string& b);

	// Flow text into lines of width up to @width.
	std::vector<std::string> FlowText(const std::string& src, size_t width);

	// Implementation of std::optional.
	template <class T>
	class optional {
	public:
		operator bool() const { return has_value_; }
		constexpr const T& operator*() const& { return value_; }
		constexpr const T* operator->() const& { return &value_; }
		optional<T>& operator=(const T& value) {
			value_ = value;
			has_value_ = true;
			return *this;
		}
		void reset() { has_value_ = false; }
		T value_or(const T& def) const { return has_value_ ? value_ : def; }

	private:
		T value_;
		bool has_value_ = false;
	};

	// Apply moves
	void apply(Position &pos, std::shared_ptr<Variation> md);

	// Unapply moves
	void unapply(Position &pos, std::shared_ptr<Variation> md);

	// Get line
	std::string get_line(std::shared_ptr<Variation> md);

	// Print line
	void print_line(std::shared_ptr<Variation> md);

	// Join line
	void join(
		std::shared_ptr<Variation> md,
		std::shared_ptr<Variation> md_after,
		Move move);

	// End of line
	std::shared_ptr<Variation> end(std::shared_ptr<Variation> md);

	// Start of line
	std::shared_ptr<Variation> begin(std::shared_ptr<Variation> md);

	// Size of line
	size_t size(std::shared_ptr<Variation> md);

	// Exception.
	class Exception : public std::runtime_error {
	public:
		Exception(const std::string& what) : std::runtime_error(what) {
		}
	};

};

#endif
