#pragma once

#include <iostream>
#include <string>
#include <sstream>

#include "position_searcher.h"
#include "position.h"
#include "utils.h"

class UciCommand
{
public:
	UciCommand() :fen("") {}
	void process(std::string command);
	bool process_part(std::string command_part);

private:
	enum State
	{
		PositionCmd = 0,
		PositionFromFen = 1,
		PositionFromStart = 2,
		PositionWithMoves = 4,
		PositionReady = 8,
		SearchGo = 16,
		SearchDepth = 32,
	};

	State state;
	std::string fen;
	std::string moves;
	int search_depth;
	Position position;
	PositionSearcher searcher;
};