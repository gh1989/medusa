#include "position.h"
#include "move_selector.h"
#include "eval.h"

#include <algorithm>

MoveSelector::MoveSelector(Position& position, bool include_quiet)
{
	auto unordered_moves  = position.legal_moves();
	auto colour = position.colour_to_move();

	for (auto &move : unordered_moves)
	{
		// Move promise score
		int promise = 0;
		
		
		// === Apply ===
		move.apply( position );
		
		// Exit the loop if actually we are quiescent and this
		// is a quiet move.
		bool is_capture		= move.is_capture();
		bool is_check		= position.in_check();

		if (!include_quiet && !(is_check || is_capture))
		{
			// Skip! Remember to unapply the move.
			move.unapply(position);
			continue;
		}
		
		if (is_check)
		{
			if (position.is_checkmate())
			{
				move.unapply(position);
				moves.insert({ -1024*PROMISE_CAPTURE*QUEEN_SCORE, move });
				break;
			}
			else
				promise += PROMISE_CHECK;
		}
		/*
	    // Static exchange eval
		if (is_capture)
		{
			promise += see(position, move);
		}
		
		// Does this move look good?
		promise += Eval::move_promise(position, move);	
		*/	
		// === Unapply ===
		move.unapply(position);	   
		moves.insert( { -promise, move } );
	};
}

int MoveSelector::see(Position &pos, Move move)
{
	int value = 0;
	
	// Get smallest attacker capture
	auto next_moves = pos.legal_moves();
	auto remove_condition = [](const Move& m){ return !m.is_capture();};
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
		int attacker = values[next_move.attacker()];
		if (attacker <= smallest_attacker)
		{
			smallest_attacker = attacker;
			next_move = m;
		}
	}
	
	// === Apply ====
	next_move.apply(pos);
	int capture_value = values[next_move.captured()] - see(pos, next_move);
	next_move.unapply(pos);
	// === unapply ====

	value = capture_value > 0 ? capture_value : 0;
	return value;
}