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
		
		// Exit the loop if actually we are quiescent and this
		// is a quiet move.
		bool is_capture = position.is_capture(move);

		// === Apply ===
		position.apply(move);
		
		bool is_check		= position.in_check();

		if (!include_quiet && !(is_check || is_capture))
		{
			// Skip! Remember to unapply the move.
			position.unapply(move);
			continue;
		}
		
		if (is_check)
		{
			if (position.is_checkmate())
			{
				position.unapply(move);
				moves.insert({ -999999999, move });
				break;
			}
			else
				promise += 100000;
		}
		
		// === Unapply ===
		position.unapply(move);
		moves.insert( { -promise, move } );
	};
}

int MoveSelector::see(Position &pos, MoveTiny move)
{
	int value = 0;
	
	// Get smallest attacker capture
	auto next_moves = pos.legal_moves();
	auto remove_condition = [pos](MoveTiny m){ return pos.is_capture(m); };
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
		int attacker = values[ pos.attacker(next_move) ];
		if (attacker <= smallest_attacker)
		{
			smallest_attacker = attacker;
			next_move = m;
		}
	}
	
	// === Apply ====
	pos.apply(next_move);
	int capture_value = values[ pos.captured( next_move ) ] - see(pos, next_move);
	pos.unapply(next_move);
	// === unapply ====

	value = capture_value > 0 ? capture_value : 0;
	return value;
}