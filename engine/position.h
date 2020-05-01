#ifndef position_h
#define position_h

#include <array>
#include <functional>
#include <memory>
#include <vector>
#include <set>
#include <iostream>

#include "bitboard.h"
#include "board.h"
#include "types.h"

namespace Medusa {

	enum MoveType
	{
		Any = 0,
		Capture = 1,
	};

	class Position
	{
	public:

		Position()
		{
			to_move = Colour::WHITE;
			plies = 0;
			castling = Castling::ALL;
			castling_reflect = false;
		}

		Position(
			const std::array<std::array<Bitboard, 6>, 2> bitboards_,
			unsigned short plies_,
			unsigned short fifty_counter_,
			Castling castling_,
			Bitboard enpassant_,
			Colour to_move_,
			bool castling_reflect_)
			:
			bitboards(bitboards_),
			plies(plies_),
			fifty_counter(fifty_counter_),
			castling(castling_),
			enpassant(enpassant_),
			to_move(to_move_),
			castling_reflect(castling_reflect_)
		{
		}

		void Apply(Move move);
		void Unapply(Move move);
		void PrettyPrint() const;

		Bitboard Occupants() const
		{
			return Occupants(Colour::WHITE) | Occupants(Colour::BLACK);
		}

		Bitboard Occupants(Colour colour) const
		{
			int index = colour.Index();
			return Occupants(index);
		}

		Bitboard Occupants(int index) const
		{
			auto coloured_occupants_bitboards = bitboards[index];
			Bitboard ret(0);

			for (int i = 0; i < Piece::NUMBER_PIECES; ++i)
			{
				ret = ret | coloured_occupants_bitboards[i];
			}

			return ret;
		}

		Position Reflect() const;

		bool MoveWasCapture(Move move) const;

		void MovePiece(Colour colour, Piece piece, Square start, Square finish)
		{
			int index = colour.Index();
			auto piece_bitboard = bitboards[index][piece];
			if (!IsOn(piece_bitboard, start) || IsOn(piece_bitboard, finish))
			{
				throw("Tried to turn off/on bit which is on/off.");
			}

			piece_bitboard = BitMove(piece_bitboard, start, finish);
			bitboards[index][piece] = piece_bitboard;
		}

		void AddPiece(Colour colour, Piece piece, Square square)
		{
			int index = colour.Index();
			auto piece_bitboard = OnBit(bitboards[index][piece], square);
			bitboards[index][piece] = piece_bitboard;
		}

		void RemovePiece(Colour colour, Piece piece, Square square)
		{
			int index = colour.Index();
			auto piece_bitboard = OffBit(bitboards[index][piece], square);
			bitboards[index][piece] = piece_bitboard;
		}

		unsigned short GetFiftyCounter() const { return fifty_counter; }
		unsigned short GetPlies() const { return plies; }

		Castling GetCastling() const
		{
			auto c = castling;
			if (castling_reflect)
				c = static_cast<Castling>((c % 4 << 2) + (c / 4));
			return c;
		}
		void SetCastling(Castling castling_) { castling = castling_; }
		void SetFiftyCounter(short fifty_counter_) { fifty_counter = fifty_counter_; }
		void SetEnPassant(Bitboard enpassant_) { enpassant = enpassant_; }
		void ApplyUCI(std::string move_str);
		Piece PieceAtSquare(Square square) const;
		bool MoveIsCapture(Move move) const;
		Piece GetAttacker(Move move) const;
		Piece Captured(Move move) const;

		void TickForward(
#ifdef _DEBUG
			std::string move_str
#endif
		)
		{
			to_move = ~to_move;
			plies++;
#ifdef _DEBUG
			past_moves.push_back(move_str);
#endif

		}

		void TickBack()
		{
			to_move = ~to_move;
			plies--;
#ifdef _DEBUG
			past_moves.pop_back();
#endif
		}

		void SetPlies(int plies_)
		{
			plies = plies_;
		}

		void DisableCastling(Colour colour)
		{
			Castling w_castling = static_cast<Castling>(Castling::W_QUEENSIDE | Castling::W_KINGSIDE);
			Castling b_castling = static_cast<Castling>(Castling::B_QUEENSIDE | Castling::B_KINGSIDE);
			Castling disable_this = colour.IsWhite() ? w_castling : b_castling;
			DisableCastling(disable_this);
		}

		void DisableCastling(Castling castle_disable)
		{
			castling = static_cast<Castling>(castling & ~castle_disable);
		}

		Colour ToMove() const
		{
			return to_move;
		}

		bool ThreeMoveRepetition() const;

		bool LastMoved(Piece piece, Square square) const;

		bool operator==(const Position& other) const {
			bool equal = true;
			equal &= castling  == other.castling;
			equal &= enpassant == other.enpassant;
			equal &= to_move.Index() == other.to_move.Index();
			equal &= bitboards == other.bitboards;
			return equal;
		}

		bool operator!=(const Position& other) const { return !operator==(other); }

		template <MoveType MT>
		std::vector<Move> LegalMoves()
		{
			auto psuedo_legal = PseudoLegalMoves<MT>();
			bool check_d = IsInCheck();
			auto pred = [this, check_d](Move move) {return this->IsIllegalMove(move, check_d);  };
			auto to_remove = std::remove_if(psuedo_legal.begin(), psuedo_legal.end(), pred);
			psuedo_legal.erase(to_remove, psuedo_legal.end());
			return psuedo_legal;
		}

		bool AnyLegalMove();

		Bitboard PieceBoard(Colour colour, Piece piece) const
		{
			int index = colour.Index();
			return bitboards[index][piece];
		}

		// Just want to deal with an enum in eval...
		Bitboard PieceBoard(int index, Piece piece) const
		{
			return bitboards[index][piece];
		}

		template <MoveType MT>
		std::vector<Move> PseudoLegalMoves();
		bool IsIllegalMove(Move move, bool check_discovered_);

		template <Piece piece, MoveType MT>
		std::vector<Move> LegalJumperMoves(
			Colour colour,
			const Bitboard* attacks) const;

		template <Piece piece, MoveType MT>
		std::vector<Move> LegalSliderMoves(
			Colour colour,
			const std::pair<int, int> *directions) const;

		template <MoveType MT>
		std::vector<Move> LegalPawnMoves(Colour colour) const;

		template <MoveType MT>
		std::vector<Move> LegalCastlingMoves(Colour colour) const;
		bool IsSquareAttacked(const Bitboard& square, Colour colour) const;
		void set_colour(Colour colour) { to_move = colour; }
		bool IsInCheck() const;
		bool IsCheckmate();
		
	private:
		std::array<std::array<Bitboard, 6>, 2> bitboards;
		Castling castling;
		unsigned short fifty_counter;
		unsigned short plies;
		Bitboard enpassant;
		Colour to_move;

#ifdef _DEBUG
		std::vector<std::string> past_moves;
#endif
		mutable bool castling_reflect;
	};

	class PositionHistory
	{
	public:
		static std::vector<Position> history;

		static void Clear()
		{
			history.clear();
		}

		static void Push(const Position& position)
		{
			history.emplace_back(position);
		}

		// Was this piece moved to this square in the last move?
		static bool LastMoved(Piece piece, Square square)
		{
			// Get the index of the history 2 plies ago.
			int idx = history.size() - 3;
			if (idx < 0)
				return false;

			// Get the piece bitboard from 4 plies ago.
			auto to_move = history.back().ToMove();
			auto piecebb = history[idx].PieceBoard(to_move, piece);

			// If we don't have the piece here then we moved it.
			return !(piecebb & SqrBb(square));
		}

		static Position Pop()
		{
			Position ret = history.back();
			history.pop_back();
			return ret;
		}

		static bool HaveBeenThreeRepetitions()
		{
			int idx = history.size() - 3;
			if (idx < 2)
				return false;

			const auto& last = history.back();
			
			// Is only a draw on the last position, can only be 
			// the same position if its the same players turn.
			int reps = 1;
			do 
			{
				if (history[idx] == last)
					reps++;
				idx = idx - 2;
			} while (idx > 0 && reps < 3);

			return reps >= 3;	
		}
	};

	inline bool Position::LastMoved(Piece piece, Square square) const
	{
		// 2 plies ago was the piece still on this square?
		return PositionHistory::LastMoved(piece, square);
	}

	inline bool Position::ThreeMoveRepetition() const
	{
		return PositionHistory::HaveBeenThreeRepetitions();
	}


	template <MoveType MT>
	inline std::vector<Move> Position::PseudoLegalMoves()
	{
		Colour us = to_move;
		std::vector<Move> moves;

		// There are not going to be more than 64 legal moves in a standard game.
		moves.reserve(64);

		// Get all moves
		auto rook_moves = LegalSliderMoves<ROOK, MT>(us, rook_directions);
		auto bishop_moves = LegalSliderMoves<BISHOP, MT>(us, bishop_directions);
		auto queen_1 = LegalSliderMoves<QUEEN, MT>(us, bishop_directions);
		auto queen_2 = LegalSliderMoves<QUEEN, MT>(us, rook_directions);
		auto knight_moves = LegalJumperMoves<KNIGHT, MT>(us, knight_attacks);
		auto king_moves = LegalJumperMoves<KING, MT>(us, neighbours);
		auto king_castling = LegalCastlingMoves<MT>(us);
		auto pawn_moves = LegalPawnMoves<MT>(us);

		// Insert them all at the end
		moves.insert(moves.end(), pawn_moves.begin(), pawn_moves.end());
		moves.insert(moves.end(), rook_moves.begin(), rook_moves.end());
		moves.insert(moves.end(), bishop_moves.begin(), bishop_moves.end());
		moves.insert(moves.end(), queen_1.begin(), queen_1.end());
		moves.insert(moves.end(), queen_2.begin(), queen_2.end());
		moves.insert(moves.end(), king_moves.begin(), king_moves.end());
		moves.insert(moves.end(), knight_moves.begin(), knight_moves.end());
		moves.insert(moves.end(), king_castling.begin(), king_castling.end());

		return moves;
	}


	template <Piece piece, MoveType MT>
	inline std::vector<Move> Position::LegalJumperMoves(
		Colour us,
		const Bitboard* attacks) const
	{
		std::vector<Move> moves;
		Bitboard piecebb = bitboards[us.Index()][piece];
		Bitboard ours = Occupants(us);
		Bitboard theirs = Occupants(~us);
		for (auto it = piecebb.begin(); it != piecebb.end(); it.operator++())
		{
			Square sqr = Square(*it);
			;			Bitboard square = squares[sqr];
			auto attack = attacks[sqr] & (~ours);
			if (MT == Capture)
				attack &= theirs;
			for (auto it2 = attack.begin(); it2 != attack.end(); it2.operator++())
			{
				Square sqr2 = Square(*it2);
				auto move = CreateMove(Square(sqr), Square(sqr2));
				moves.emplace_back(move);
			}
		}
		return moves;
	}

	template <Piece piece, MoveType MT>
	inline std::vector<Move> Position::LegalSliderMoves(Colour us, const std::pair<int, int> *directions) const
	{
		std::vector<Move> moves;
		Colour them = ~us;
		Bitboard piecebb = bitboards[us.Index()][piece];
		Bitboard theirs = Occupants(them);
		Bitboard ours = Occupants(us);
		for (auto it = piecebb.begin(); it != piecebb.end(); it.operator++())
		{
			Square sqr = Square(*it);
			Bitboard square = squares[sqr];

			for (int i = 0; i < 4; i++)
			{
				auto dir = directions[i];
				int sidx = sqr;
				while (true)
				{
					int rank = sidx / 8;
					if (dir.first > 0 && rank == 7)
						break;
					if (dir.first < 0 && rank == 0)
						break;
					int file = sidx % 8;
					if (dir.second > 0 && file == 7)
						break;
					if (dir.second < 0 && file == 0)
						break;

					sidx += 8 * dir.first + dir.second;
					if (sidx > 63 | sidx < 0)
						break;
					Bitboard new_bb = squares[sidx];
					if (new_bb&ours)
						break;
					auto sqrto = Square(sidx);

					// Skip non-captures.
					auto move = CreateMove(Square(sqr), Square(sidx));
					if (MT == Capture && !(theirs & squares[sidx]))
						continue;

					moves.emplace_back(move);
					if (new_bb&theirs)
						break;
				}
			}
		}

		return moves;
	}

	template <MoveType MT>
	inline std::vector<Move> Position::LegalPawnMoves(Colour colour) const
	{
		// Work out on the pawn moves based on the reflected position.
		Position context(*this);
		if (colour.IsBlack())
			context = context.Reflect();

		// Promote pieces
		const Piece promote_pieces[] = {
			KNIGHT,
			BISHOP,
			ROOK,
			QUEEN
		};

		std::vector<Move> moves;
		moves.reserve(64);

		// Get the position information from the potentially reflected position.
		Bitboard pawns = context.bitboards[colour.Index()][PAWN];
		Bitboard occ = context.Occupants();
		Bitboard bocc = context.Occupants(~colour);

		for (auto it = pawns.begin(); it != pawns.end(); it.operator++())
		{
			auto sqr = Square(*it);
			int rank = sqr / 8;
			Bitboard square = squares[sqr];

			// Pushes
			Bitboard push_once = square << 8;
			if (!(push_once & occ) && MT != Capture)
			{
				if (rank == 1) // 2nd rank, double push.
				{
					Bitboard push_twice = square << 16;
					if (!(push_twice & occ))
						moves.emplace_back(CreateMove(sqr, Square(sqr + 16)));
				}
				if (rank == 6) // 7th rank, promotion
					for (auto& prom : promote_pieces)
						moves.emplace_back(CreatePromotion(sqr, Square(sqr + 8), prom));
				else
					moves.emplace_back(CreateMove(sqr, Square(sqr + 8)));
			}

			// Captures
			if (pawn_attacks[sqr] & (bocc | enpassant))
				for (int shft : {7, 9})
				{
					// You cannot capture off the side of the board.
					int file = sqr % 8;
					if (file == 0 && shft == 7)
						continue;
					if (file == 7 && shft == 9)
						continue;

					Bitboard capt_diag = square << shft;
					Square to = Square(sqr + shft);
					if (capt_diag & bocc)
					{
						if (rank == 6) // 7th rank, promotion
							for (auto& prom : promote_pieces)
								moves.emplace_back(CreatePromotion(sqr, to, prom));
						else
						{
							// Taking enpassant
							if (pawn_attacks[sqr] & enpassant)
								moves.emplace_back(CreateEnPassant(sqr, to));
							else
								moves.emplace_back(CreateMove(sqr, to));
						}
					}
				}
		}

		// Generate moves on the reflected position for convenience, so return
		// the reflected moves to be correct.
		if (colour.IsBlack())
		{
			std::transform(
				moves.begin(),
				moves.end(),
				moves.begin(),
				[](Move move) { return ReflectMove(move); });
		}

		return moves;
	}

	template <MoveType MT>
	inline std::vector<Move> Position::LegalCastlingMoves(Colour colour) const
	{
		std::vector<Move> moves;
		if (MT == Capture)
			return moves;

		bool black = colour.IsBlack();
		auto castling = GetCastling();
		auto all_occupants = Occupants();

		Colour them = ~colour;

		auto queenside = black ? B_QUEENSIDE : W_QUEENSIDE;
		auto kingside = black ? B_KINGSIDE : W_KINGSIDE;
		bool reverse_pawn_attacks = !black;
		bool checked = IsInCheck();

		// queenside castling
		if (bool(castling & queenside) && !checked)
		{
			auto near_square = squares[black ? d8 : d1];
			auto extra_square = squares[black ? b8 : b1];
			auto far_square = squares[black ? c8 : c1];
			bool unobstructed = !(all_occupants & (near_square | far_square | extra_square));
			if (unobstructed)
			{
				// Want to delay this expensive stuff.
				unobstructed &= !IsSquareAttacked(near_square, them);
				unobstructed &= !IsSquareAttacked(far_square, them);
				if (unobstructed)
				{
					auto move = CreateCastle(black ? e8 : e1, black ? c8 : c1);
					moves.push_back(move);
				}
			}
		}

		// kingside castling
		if (bool(castling & kingside) && !checked)
		{
			auto near_square = squares[black ? f8 : f1];
			auto far_square = squares[black ? g8 : g1];
			bool unobstructed = !(all_occupants & (near_square | far_square));
			if (unobstructed)
			{
				unobstructed &= !IsSquareAttacked(near_square, them);
				unobstructed &= !IsSquareAttacked(far_square, them);
				if (unobstructed)
				{
					auto move = CreateCastle(black ? e8 : e1, black ? g8 : g1);
					moves.push_back(move);
				}
			}
		}

		return moves;
	}
}

#endif
