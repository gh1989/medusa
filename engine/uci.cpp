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
	bool end = command_part.empty();

	// Go
	if (command_part == "go")
	{
		state = SearchGo;
		return true;
	}

	// Go body
	if (state == SearchGo)
	{
		// Go-depth
		if (command_part == "depth")
		{
			state = SearchDepth;
			return true;
		}

		// Go-end
		if (end)
		{
			searcher.search(position, search_depth);
			return false;
		}

		return false;
	}

	// Go-depth
	if (state == SearchDepth)
	{
		search_depth = std::stoi(command_part);
		state = SearchGo;
		return true;
	}

	// Position
	if (command_part == "position")
	{
		state = PositionCmd;
		return true;
	}
	
	// Position-end
	if (end && (state & (PositionFromStart | PositionFromFen) ) )
	{
		position = position_from_fen(fen);

		// End of command.
		return false;
	}
	if (end)
		return false;

	// Position body
	switch (state)
	{
	case PositionCmd:
	{
		// Position-startpos
		fen = "";
		if (command_part == "startpos")
		{
			state    = PositionFromStart;
		}
		// Position-fen
		else if (command_part == "fen")
			state = PositionFromFen;
		else
			return false; // failed
		break;
	}
	case PositionFromStart:
	{
		// position starpos moves
		if (command_part == "moves")
		{
			position = position_from_fen(fen);
			state = PositionWithMoves;
		}
		else
			return false;
		break;
	}
	case PositionWithMoves:
	{
		// position [ startpos | fen ] moves ... 
		position.apply_uci(command_part);
		break;
	}
	// position fen ...
	case PositionFromFen:
	{
		// position fen ...
		if (command_part != "moves")
		{
			if (fen.empty())
				fen = command_part;
			else
				fen = fen + " " + command_part;
		}
		else
		{
			// position fen moves
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
