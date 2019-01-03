#ifndef types_h
#define types_h

#include <stdexcept>
#include <stdint.h>
#include <string>

class Colour
{
public:

	Colour()
	{
		value = 1;
	}

	Colour(int value_)
	{
		if (abs(value_) != 1)
			throw("Invalid colour");
		value = value_;
	}

	int operator*(int other) const
	{
		return value * other;
	}

	Colour operator~() const
	{
		return Colour(-value);
	}

	std::string to_string() const
	{
		return value < 0 ? "B" : "W";
	}

	bool is_black() const
	{
		return value < 0;
	}

	bool is_white() const
	{
		return !is_black();
	}

	// Data index
	int index() const
	{
		return (value < 0 ? 1 : 0);
	}

	int plies() const
	{
		return (value < 0 ? 1 : 0);
	}

	static Colour from_string(std::string colour_string)
	{
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

/*
* Necessary hack to get c++11 enums to work 
*/
#ifdef SWIG
%rename(Piece) PieceNS;
#endif
struct PieceNS
{
	static const std::string PIECE_STRINGS;
	enum Value { KNIGHT, BISHOP, ROOK, QUEEN, KING, PAWN, NUMBER_PIECES, NO_PIECE = -1 };

};
typedef PieceNS::Value Piece; 

/*
* Necessary hack to get c++11 enums to work 
*/
#ifdef SWIG
%rename(Castling) CastlingNS;
#endif

struct CastlingNS
{

  enum Value {
	  W_QUEENSIDE = 1,
	  W_KINGSIDE = 2,
	  B_QUEENSIDE = 4,
	  B_KINGSIDE = 8,
	  ALL = 15
	};
};

typedef CastlingNS::Value Castling; 

template <typename Type, int N>
struct w_array {
	Type data[N];

	size_t __len__() const { return N; }

	const Type& __getitem__(size_t i) const throw(std::out_of_range) 
	{
	    if (i >= N || i < 0)
	      throw std::out_of_range("out of bounds access");
	    return this->data[i];
	}

	void __setitem__(size_t i, const Type& v) throw(std::out_of_range) 
	{
	    if (i >= N || i < 0)
	      throw std::out_of_range("out of bounds access");
	    this->data[i] = v;
	}

	w_array<int, N> operator+(const w_array<int, N> other) const
	{
		auto ret = *this;
		for (int i = 0; i < N; i++)
		{
			ret.data[i] = data[i] + other.data[i];
		}
		return ret;
	}

	w_array<int, N> operator-(const w_array<int, N> other) const
	{
		auto ret = *this;
		for (int i = 0; i < N; i++)
		{
			ret.data[i] = data[i] - other.data[i];
		}
		return ret;
	}

	w_array<int, N> operator*(const w_array<int, N> other) const
	{
		auto ret = *this;
		for (int i = 0; i < N; i++)
		{
			ret.data[i] = data[i] * other.data[i];
		}
		return ret;
	}

	w_array<int, N> clip_values(int lower, int upper) const
	{
		auto ret = *this;
		int di;
		for (int i = 0; i < N; i++)
		{
			di = data[i];
			di = upper < di ? upper : di;
			di = lower > di ? lower : di;
			ret.data[i] = di;
		}
		return ret;
	}

	template<int M, typename std::enable_if<M*M==N>::type = 0>
	w_array<int, N> 
	operator*(const w_array<int, M> other) const
	{
		auto ret = *this;
		for (int i = 0; i < N; i++)
		{
			ret.data[i] = data[i] * other.data[i % M];
		}
		return ret;
	}

	int sum() const
	{
		int ret = 0;
		for (int i = 0; i < N; i++)
		{
			ret += data[i];
		}
		return ret;
	}
};

// TODO: move this.
std::string piece_string_lower(Piece piece);

#endif