#include "utils.h"
#include "types.h"

#include <stdint.h>
#include <iomanip>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cctype>

namespace medusa
{
	// Piece strings
	const std::string PIECE_STRINGS = "NBRQKP";

	const std::string piece_strings[6] = { "n", "b", "r", "q", "k", "p" };

	std::string piece_string_lower(Piece piece)
	{
		return piece_strings[piece];
	}

	std::string piece_string(Piece piece, Colour c)
	{
		auto str = PIECE_STRINGS[piece];
		if (c.is_black())
			str = ::tolower(str);
		return std::string(1, str);
	}

	Bitboard bitboard_from_string(std::string str)
	{
		if (str[0] < 'a' || str[1] < '1' || str[0] > 'h' || str[1] > '8')
			throw std::runtime_error("Square string is formatted improperly.");
		uint64_t boardnum = str[0] - 'a' + 8 * (str[1] - '1');
		return Bitboard(1ULL << boardnum);
	}

	Position position_from_fen(std::string fen)
	{
		const std::string castle_enum = "QKqk";

		if (fen == "")
			fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

		std::vector<std::string> strings;
		std::istringstream f(fen);
		std::string s;

		while (std::getline(f, s, ' ')) {
			strings.push_back(s);
		}

		if (strings.size() != 6)
		{
			throw std::runtime_error("FEN is formatted improperly.");
		}

		auto pos = Position(); // empty board
		auto posstring = strings[0];
		auto colour_string = strings[1];
		auto castlestring = strings[2];
		auto epstring = strings[3];
		auto fiftycounter = strings[4];
		auto moveclock = strings[5];

		auto ite = 8;
		unsigned short sub_ite, found;

		std::istringstream f2(posstring);
		strings.clear();
		while (std::getline(f2, s, '/')) {
			sub_ite = 0;
			for (char& c : s) {
				if (c >= '1' && c <= '9')
				{
					sub_ite += c - '0';
				}
				else
				{
					const std::string piece_strings = "NBRQKP";
					found = piece_strings.find(::toupper(c));
					if (found == std::string::npos)
					{
						throw std::runtime_error("FEN is formatted improperly.");
					}
					auto piece = static_cast<Piece>(found);
					auto colour = (::toupper(c) == c) ? Colour::WHITE : Colour::BLACK;
					auto square = static_cast<Square>(8 * (ite - 1) + sub_ite);
					pos.add_piece(colour, piece, square);
					sub_ite++;
				}
			}
			if (sub_ite != 8)
			{
				throw std::runtime_error("FEN is formatted improperly.");
			}

			ite -= 1;
		}

		if (ite != 0)
		{
			throw std::runtime_error("FEN is formatted improperly.");
		}

		if (castlestring != "-")
		{
			int castlesum = 0;
			for (char c : castlestring)
			{
				found = castle_enum.find(c);
				if (found != std::string::npos)
					castlesum += 1 << found;
			}

			pos.set_castling(static_cast<Castling>(castlesum));
		}
		else
		{
			pos.set_castling(static_cast<Castling>(0));
		}

		auto colour = Colour::from_string(colour_string);
		pos.set_colour(colour);

		if (epstring != "-")
		{
			auto epboard = bitboard_from_string(epstring);
			pos.set_enpassant(epboard);
		}
		else
		{
			pos.set_enpassant(0);
		}

		if (fiftycounter != "-")
		{
			unsigned short counter = std::stoi(fiftycounter);
			if (counter >= 10000)
				throw std::runtime_error("FEN is formatted improperly.");

			pos.set_fifty_counter(counter);
		}
		else
		{
			pos.set_fifty_counter(0);
		}

		if (moveclock != "-")
		{
			unsigned short counter = (std::stoi(moveclock) - 1);
			if (counter > 10000)
				throw std::runtime_error("FEN is formatted improperly.");

			pos.set_plies(2 * counter + colour.plies());
		}

		return pos;
	}
	
	// Get square name as string
	std::string square_name(Square sqr)
	{
		const std::string file_strings[8] = { "a", "b", "c", "d", "e", "f", "g", "h" };
		const std::string rank_strings[8] = { "1", "2", "3", "4", "5", "6", "7", "8" };
		int square_int = static_cast<int>(sqr);
		return file_strings[square_int % 8] + rank_strings[int(square_int / 8)];
	}


	// Print the board as ASCII art.
	void Position::pp() const
	{
		// collect information
		std::string pos[8][8];
		for (int i = 0; i < 8; i++)
			for (int j = 0; j < 8; j++)
				pos[i][j] = ' ';

		for (auto clr : { Colour::WHITE, Colour::BLACK })
		{
			auto clr_bitboard = bitboards[clr.index()];
			for (int i = 0; i < NUMBER_PIECES; ++i)
			{
				auto occup = clr_bitboard[i];
				Piece piece = static_cast<Piece>(i);
				std::string modif = piece_string(piece, clr);
				for (int i=0; i<63; i++)
				{
					int j = (7 - int(i / 8)) % 8;
					int k = i % 8;
					if (clr_bitboard[piece] & squares[i])
						pos[j][k] = modif;
				}
			}
		}

		// print out the board
		std::string baseline = "+---";
		for (auto j = 0; j < 7; j++)
			baseline += "+---";
		baseline += "+\n";

		std::string output = baseline;
		for (auto i = 0; i < 8; i++)
		{
			for (auto j = 0; j < 8; j++)
				output += "| " + pos[i][j] + " ";
			output += "|\n";
			output += baseline;
		}

		std::cout << output;
		Bitboard ep = enpassant;

		if (ep)
		{
			std::cout << "en-passant: ";
			for (int i = 0; i < 63; i++)
			{
				if ( squares[i] & enpassant )
					std::cout << square_name(Square(i));
			}
			std::cout << std::endl;
		}
		std::cout << "fiftycounter: " << fifty_counter << std::endl;
		int castlerights = castling;
		const std::string crights = "QKqk";
		std::cout << "castlerights: " << castlerights << " ";
		for (char c : crights)
		{
			if (castlerights % 2)
				std::cout << c;
			castlerights /= 2;
		}

		std::cout << std::endl;
		auto clr = colour_to_move();
		std::cout << "plies: " << plies << std::endl;
		std::cout << "colour to move: " << clr.to_string() << std::endl;
	}

	// Apply UCI move to the position.
	void Position::apply_uci(std::string move_str)
	{
		auto moves = legal_moves();

		for (auto m : moves)
		{
			auto this_move_str = as_uci(m);
			if (this_move_str == move_str)
			{
				apply(m);
				return;
			}
		}

		std::cout << "Warning: attempted to apply illegal move: " << move_str <<std::endl;
	}

	std::chrono::time_point<std::chrono::system_clock> SteadyClockToSystemClock(
		std::chrono::time_point<std::chrono::steady_clock> time) {
		return std::chrono::system_clock::now() +
			std::chrono::duration_cast<std::chrono::system_clock::duration>(
				time - std::chrono::steady_clock::now());
	}

	std::string FormatTime(
		std::chrono::time_point<std::chrono::system_clock> time) {
		std::ostringstream ss;
		using namespace std::chrono;
		auto us =
			duration_cast<microseconds>(time.time_since_epoch()).count() % 1000000;
		auto timer = std::chrono::system_clock::to_time_t(time);
		/*
		ss << std::put_time(std::localtime(&timer), "%m%d %T") << '.'
			<< std::setfill('0') << std::setw(6) << us;
		*/
		return ss.str();
	}

	std::string StrJoin(const std::vector<std::string>& strings,
		const std::string& delim) {
		std::string res;
		for (const auto& str : strings) {
			if (!res.empty()) res += delim;
			res += str;
		}
		return res;
	}

	std::vector<std::string> StrSplitAtWhitespace(const std::string& str) {
		std::vector<std::string> result;
		std::istringstream iss(str);
		std::string tmp;
		while (iss >> tmp) result.emplace_back(std::move(tmp));
		return result;
	}

	std::vector<std::string> StrSplit(const std::string& str,
		const std::string& delim) {
		std::vector<std::string> result;
		for (std::string::size_type pos = 0, next = 0; pos != std::string::npos;
			pos = next) {
			next = str.find(delim, pos);
			result.push_back(str.substr(pos, next - pos));
			if (next != std::string::npos) next += delim.size();
		}
		return result;
	}

	std::vector<int> ParseIntList(const std::string& str) {
		std::vector<int> result;
		for (const auto& x : StrSplit(str, ",")) {
			result.push_back(std::stoi(x));
		}
		return result;
	}

	std::string LeftTrim(std::string str) {
		auto it = std::find_if(str.begin(), str.end(),
			[](int ch) { return !std::isspace(ch); });
		str.erase(str.begin(), it);
		return str;
	}

	std::string RightTrim(std::string str) {
		auto it = std::find_if(str.rbegin(), str.rend(),
			[](int ch) { return !std::isspace(ch); });
		str.erase(it.base(), str.end());
		return str;
	}

	std::string Trim(std::string str) {
		return LeftTrim(RightTrim(std::move(str)));
	}

	bool StringsEqualIgnoreCase(const std::string& a, const std::string& b) {
		return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char a, char b) {
			return std::tolower(a) == std::tolower(b);
		});
	}

	std::vector<std::string> FlowText(const std::string& src, size_t width) {
		std::vector<std::string> result;
		auto paragraphs = StrSplit(src, "\n");
		for (const auto& paragraph : paragraphs) {
			result.emplace_back();
			auto words = StrSplit(paragraph, " ");
			for (const auto& word : words) {
				if (result.back().empty()) {
					// First word in line, always add.
				}
				else if (result.back().size() + word.size() + 1 > width) {
					// The line doesn't have space for a new word.
					result.emplace_back();
				}
				else {
					// Appending to the current line.
					result.back() += " ";
				}
				result.back() += word;
			}
		}
		return result;
	}
}
