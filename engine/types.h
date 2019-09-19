#ifndef types_h
#define types_h

#include <functional>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <vector>


namespace medusa
{
	// Move (16 bit int)
	typedef unsigned short Move;
		
	// Variation
	class Variation
	{
	public:
		Variation() : move(0) {};
		Variation(const Variation&) = default;
		Variation(Variation&&) = default;
		Variation& operator=(const Variation&) = default;
		Variation& operator=(Variation&&) = default;
		~Variation() = default;

		std::shared_ptr<Variation> before;
		std::shared_ptr<Variation> after;
		Move move;
	};
	
	// Special moves
	enum SpecialMove
	{
		NONE = 0,
		CAPTURE_ENPASSANT = 1,
		CASTLE = 2,
		PROMOTE = 3,
	};

	// Score 
	class Score
	{
	public:
		Score();
		static Score Checkmate(size_t number);
		static Score Centipawns(int centipawns, int depth);
		static Score Infinite();
		bool operator==(const Score& rhs) const;
		bool operator<(const Score& rhs) const;
		bool operator!=(const Score& rhs) const;
		bool operator>(const Score& rhs) const;
		bool operator>=(const Score& rhs) const;
		bool operator<=(const Score& rhs) const;
		bool is_mate() const;
		int get_mate_in() const;
		double get_centipawns() const;
		int get_depth() const;
		Score operator-() const;
		Score operator-(int centipawns) const;
		Score operator+(int centipawns) const;

	private:
		bool unavoidable_mate;
		int mate_in;
		int centipawns_for;
		int depth;
		int infinite = 0;
	};

	// Principal variation info
	struct PvInfo
	{
		std::shared_ptr<Variation> pv;
		Score score;
		int nodes_per_second;
		int nodes_searched;
		int time_ms;
		int game_id = -1;
		bool is_black;
		int depth;
		int seldepth;
		int time;
		int nodes;
		int nps;
		int tb_hits;
		int multipv;

		using Callback = std::function<void(const std::vector<PvInfo>&)>;
	};

	// Best move info
	struct BestMoveInfo
	{
		Move best_move = 0;
		Score score; // careful when we do this because in the frame of the search
			         // we are always the maximizer...
		unsigned int depth = 0;

		using Callback = std::function<void(const BestMoveInfo&)>;
	};
	
	// Colour class
	class Colour
	{
	public:
		Colour() : value(1) {}
		Colour(int value_)
		{
			if (abs(value_) != 1)
				throw("Invalid colour");
			value = value_;
		}
		int operator*(int other) const { return value * other; }
		Colour operator~() const { return Colour(-value);}
		std::string to_string() const {
			return value < 0 ? "B" : "W";
		}
		bool is_black() const {	return value < 0;}
		bool is_white() const {	return !is_black();	}
		int index() const {	return (value < 0 ? 1 : 0);	}
		int plies() const {	return (value < 0 ? 1 : 0);	}
		static Colour from_string(std::string colour_string) {
			if (colour_string == "w")
				return Colour::WHITE;
			if (colour_string == "b")
				return Colour::BLACK;
			throw("Colour string formatted improperly.");
		}
		static const Colour WHITE;
		static const Colour BLACK;

	private:
		int value;
	};
};

#endif