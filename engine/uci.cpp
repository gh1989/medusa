#pragma once

#include "uci.h"

void UciCommand::process(std::string command)
{
	bool keep_going = true;
	std::istringstream iss(command);

	do {
		std::string word;
		iss >> word;
		keep_going &= process_part(word);
		if (!keep_going)
			break;

	} while (iss);
}

bool UciCommand::process_part(std::string command_part)
{
	if (command_part == "go")
	{
		int depth = 4;
		searcher.search(position, depth);

		// End of command.
		return false;
	}

	// Prepare position stuff.
	if (command_part == "position")
	{
		state = UciCommand::PositionCmd;
		return true;
	}
	
	// If we end and did not apply moves, need to create position.
	bool end = command_part.empty();
	if (end && (state & (PositionFromStart | PositionFromFen) ) )
	{
		position = position_from_fen(fen);

		// End of command.
		return false;
	}
	if (end)
		return false;

	switch (state)
	{
	case PositionCmd:
	{
		fen = "";
		if (command_part == "startpos")
		{
			state    = PositionFromStart;
		}
		else if (command_part == "fen")
			state = PositionFromFen;
		else
			return false; // failed
		break;
	}
	case PositionFromStart:
	{
		// a way to escape from fen mode.
		if (command_part == "moves")
		{
			position = position_from_fen(fen);
			state = PositionWithMoves;
		}
		else
			return false; // failed
		break;
	}
	case PositionWithMoves:
	{
		position.apply_uci(command_part);
		break;
	}
	case PositionFromFen:
	{
		if (command_part != "moves")
		{
			// keep adding to the fen, it has spaces.
			if (fen.empty())
				fen = command_part;
			else
				fen = fen + " " + command_part;
		}
		else
		{
			position = position_from_fen(fen);
			state    = PositionWithMoves;
		}
				
		break;
	}
	default:
		throw;
	}

	return true; // ok
}
