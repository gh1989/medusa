#include "moveiter.h"

namespace Medusa
{
	MoveSelector::MoveSelector(Position& position_, bool allmvs)
	{
		/// Hand crafted logic for ordering the moves currently 
		/// legal in the position. We prioritize the checks and
		/// captures to look at first, with the least valuable 
		/// piece capturing the most valuable first. 
		Position position = position_;
		auto mvs = position.LegalMoves<Medusa::Any>();
		auto turn = position.ToMove();
		
		/// The guessed constants for prioritising certain moves.
		/// the values guarantee that checks are considered before 
		/// captures but captures over other moves. It also discourages
		/// moving the same piece twice. This may be complete non-
		/// sense though and is just a placeholder for more robust 
		/// treatment in the future.
		const int _cvallst = 10;
		const int _cvalchk = 50;
		const int _cvalcap = 25;

		/// Iterate through all legal moves, there is an option to 
		/// include all moves or just checks/captures which this loop
		/// respects. If we find a checkmate, then just exit with that
		/// as the only move... that is probably not correct behaviour.
		for (auto &mv : mvs)
		{
			/// Get the attacker in the move, and which square it is 
			/// originating. The value of that piece is PAWN-attkr, check
			/// if it is a capture.
			auto attkr = position.GetAttacker(mv);
			auto mvfrm = GetFrom(mv);
			int mvval = PAWN - attkr;
			bool iscap = position.MoveIsCapture(mv);

			// Apply the move and do necessary checks
			position.Apply(mv);
			bool ischk = position.IsInCheck();
			if (!allmvs && !(ischk || iscap)) {
				position.Unapply(mv);
				continue;
			}
			// Encourage checks/captures first
			if (iscap)
				mvval += _cvalcap;
			if (ischk)
			{
				if (!position.AnyLegalMove())
				{
					position.Unapply(mv);
					moves.clear();
					moves.insert({ 0, mv });
					break;
				}
				else mvval += _cvalchk;
			}
			// Discourage same piece repeatedly.
			if (position.LastMoved(attkr, mvfrm))
				mvval -= _cvallst;

			position.Unapply(mv);
			moves.insert({ -mvval, mv });
		};
	}

	int MoveSelector::SEE(Position &pos, Move move)
	{
		int value = 0;

		// Get smallest attacker capture
		auto next_moves = pos.LegalMoves<Capture>();
		auto remove_condition = [pos](Move m) { return !pos.MoveIsCapture(m); };
		auto to_remove = std::remove_if(next_moves.begin(), next_moves.end(), remove_condition);
		next_moves.erase(to_remove, next_moves.end());

		bool capture_exists = next_moves.size();
		if (!capture_exists)
		{
			return value;
		}

		// Get the smallest attacker
		auto next_move = next_moves.back();
		int smallest_attacker = 100000;
		for (auto m : next_moves)
		{
			int attacker = Evaluation::piece_values.at(pos.GetAttacker(next_move));
			if (attacker <= smallest_attacker)
			{
				smallest_attacker = attacker;
				next_move = m;
			}
		}

		// === Apply ====
		auto captured = pos.Captured(next_move);
		pos.Apply(next_move);
		int capture_value = Evaluation::piece_values.at(captured) - SEE(pos, next_move);
		pos.Unapply(next_move);
		// === unapply ====

		value = capture_value > 0 ? capture_value : 0;
		return value;
	}
};