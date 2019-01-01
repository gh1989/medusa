#ifndef move_h
#define move_h

#include "bitboard.h"
#include "types.h"

#include <memory>
#include <string>

class Position;

class MoveImpl
{
public:
	MoveImpl(int s, int f) : start(static_cast<Bitboard::Square>(s)), finish(static_cast<Bitboard::Square>(f)) {};
	static void board_string(Bitboard);
	static std::string piece_string(Piece);
	virtual std::string as_uci() const;
	virtual std::string as_string() const {return "?";}
	virtual void apply(Position& pos) const;
	virtual void unapply(Position& pos) const;
	virtual bool described_by(std::string str) const = 0;
    virtual std::shared_ptr<MoveImpl> reflect() const = 0;
	virtual bool is_capture() const { return false; }
	Bitboard::Square get_start() const { return start;  }
	Bitboard::Square get_finish() const { return finish; }
	virtual Piece attacker() const { return Piece::NO_PIECE; }
	virtual Piece captured() const { return Piece::NO_PIECE; }
	virtual Piece promoted() const { return Piece::NO_PIECE; }
protected:
	Bitboard::Square start;
	Bitboard::Square finish;
	mutable Castling castling_before;
};

class SimpleMoveImpl : public MoveImpl
{
public:
	SimpleMoveImpl(Colour c, Piece p, int s, int f) : colour(c), piece(p), MoveImpl(s, f) {}
	std::string as_string() const override;
	void apply(Position& pos) const override;
	void unapply(Position& pos) const override;
	virtual Piece attacker() const { return piece; }

    bool described_by(std::string str) const override;
	std::shared_ptr<MoveImpl> reflect() const override;

private:
	Colour colour;
	Piece piece;
};

class CaptureMoveImpl : public MoveImpl
{
public:
	CaptureMoveImpl(Colour c, Piece p, Piece cp, int s, int f, int cp_s)
	: 
	colour(c), 
	captured_piece(cp), 
	piece(p),
	captured_start(static_cast<Bitboard::Square>(cp_s)),
	MoveImpl(s, f)
	{
		opposite_colour = ~colour;
	}
	
	CaptureMoveImpl(Colour c, Piece p, Piece cp, int s, int f) : CaptureMoveImpl(c, p, cp, s, f, f) {}
	bool is_capture() const override { return true; }
	virtual Piece attacker() const override { return piece; };
	virtual Piece captured() const override { return captured_piece; }
	std::string as_string() const override;
	void apply(Position& pos) const override;
	void unapply(Position& pos) const override;
    bool described_by(std::string str) const override;
	std::shared_ptr<MoveImpl> reflect() const override;

protected:
	Colour colour;
	Colour opposite_colour;
	Piece piece;
	Piece captured_piece;
	Bitboard::Square captured_start;	
};

class PromotionMoveImpl : public MoveImpl
{
public:
	PromotionMoveImpl(Colour c, Piece pp, int s, int f) 
	: 
	colour(c), 
	promote_piece(pp), 	
	MoveImpl(s, f)
	{}

	std::string as_string() const override;
	void apply(Position& pos) const override;
	void unapply(Position& pos) const override;
	std::string as_uci() const override;
	bool described_by(std::string str) const override;
    std::shared_ptr<MoveImpl> reflect() const override;
	virtual Piece attacker() const override { return Piece::PAWN; };
	virtual Piece promoted() const override { return promote_piece;  }

protected:
	Colour colour;
	Piece promote_piece;
};

class PromotionCaptureMoveImpl : public PromotionMoveImpl
{
public:
	PromotionCaptureMoveImpl(Colour c, Piece pp, Piece cp, int s, int f) 
	: captured_piece(cp), PromotionMoveImpl(c, pp, s, f)
	{
		opposite_colour = ~colour;
	}
	bool is_capture() const override { return true; }
	std::string as_string() const override;
	void apply(Position& pos) const override;
	void unapply(Position& pos) const override;
    bool described_by(std::string str) const override;
    std::shared_ptr<MoveImpl> reflect() const override;
	virtual Piece captured() const { return captured_piece; }

private:
	Piece captured_piece;
	Colour opposite_colour;
};

class CastleMoveImpl : public MoveImpl
{
public:
	CastleMoveImpl(Colour colour, int start_, int finish_, int rook_start_, int rook_finish_) 
	:
	colour(colour),
	rook_start(static_cast<Bitboard::Square>(rook_start_)), 
	rook_finish(static_cast<Bitboard::Square>(rook_finish_)),
	MoveImpl(start_, finish_)
	{}

	std::string as_string() const override;
	void apply(Position& pos) const override;
	void unapply(Position& pos) const override;
    bool described_by(std::string str) const override;
    std::shared_ptr<MoveImpl> reflect() const override;

private:
	Colour colour;
	Bitboard::Square rook_start, rook_finish;
	Castling castling_before;

};

class Move
{
public:
	Move() = delete;
	Move(std::shared_ptr<MoveImpl> move_impl_) : move_impl(move_impl_) {}
	bool described_by(std::string str) const { return move_impl->described_by(str); }
	void board_string(Bitboard bitboard) { move_impl->board_string(bitboard); }
	std::string piece_string(Piece piece) { move_impl->piece_string(piece); }
	std::string as_string() const { return move_impl->as_string(); }
	std::string as_uci() const { return move_impl->as_uci(); }
	void apply(Position& pos) const { move_impl->apply(pos); }
	void unapply(Position& pos) const { move_impl->unapply(pos); };
	Move reflect() const { return Move(move_impl->reflect()); };
	bool is_capture() const { return move_impl->is_capture(); };
	Piece attacker() const { return move_impl->attacker();  }
	Piece captured() const { return move_impl->captured(); }
	Piece promoted() const { return move_impl->promoted(); }
	Bitboard::Square get_start() const { return move_impl->get_start(); }
	Bitboard::Square get_finish() const { return move_impl->get_finish(); }

private:
	std::shared_ptr<MoveImpl> move_impl;
};

class MoveFactory
{
public:
	static Move Simple(Colour c, Piece p, int s, int f)
	{
		std::shared_ptr<MoveImpl> impl(new SimpleMoveImpl(c, p, s, f));
		return Move(impl);
	}

	static Move Capture(Colour c, Piece p, Piece cp, int s, int f, int cp_s)
	{
		std::shared_ptr<MoveImpl> impl(new CaptureMoveImpl(c, p, cp, s, f, cp_s));
		return Move(impl);
	}

	static Move Promotion(Colour c, Piece pp, int s, int f)
	{
		std::shared_ptr<MoveImpl> impl(new PromotionMoveImpl(c, pp, s, f));
		return Move(impl);
	}

	static Move Castle(Castling castling_)
	{
		Colour colour;
		Bitboard::Square start, finish, rook_start, rook_finish;

		switch (castling_)
		{
			case (Castling::W_QUEENSIDE):
			{
				start = Bitboard::Square::e1;
				finish = Bitboard::Square::c1;
				rook_start = Bitboard::Square::a1;
				rook_finish = Bitboard::Square::d1;
				colour = Colour::WHITE;
				break;
			}
			case (Castling::W_KINGSIDE):
			{
				start = Bitboard::Square::e1;
				finish = Bitboard::Square::g1;
				colour = Colour::WHITE;
				rook_start = Bitboard::Square::h1;
				rook_finish = Bitboard::Square::f1;
				break;
			}
			case (Castling::B_QUEENSIDE):
			{
				start = Bitboard::Square::e8;
				finish = Bitboard::Square::c8;
				rook_start = Bitboard::Square::a8;
				rook_finish = Bitboard::Square::d8;
				colour = Colour::BLACK;
				break;
			}
			case (Castling::B_KINGSIDE):
			{
				start = Bitboard::Square::e8;
				finish = Bitboard::Square::g8;
				rook_start = Bitboard::Square::h8;
				rook_finish = Bitboard::Square::f8;
				colour = Colour::BLACK;
				break;
			}
		}

		std::shared_ptr<MoveImpl> impl(new CastleMoveImpl(colour, start, finish, rook_start, rook_finish));
		return Move(impl);
	}

	static Move PromotionCapture(Colour c, Piece pp, Piece cp, int s, int f)
	{
		std::shared_ptr<MoveImpl> impl(new PromotionCaptureMoveImpl(c, pp, cp, s, f));
		return Move(impl);
	}
};

#endif
