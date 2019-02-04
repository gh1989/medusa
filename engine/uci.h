#pragma once

#include <iostream>
#include <string>
#include <sstream>

#include "position_searcher.h"
#include "position.h"
#include "utils.h"

const int DEFAULT_DEPTH = 4;

class UciCommand
{
public:
	UciCommand() :fen(""), search_depth(DEFAULT_DEPTH), state(Idle) {}
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
		Idle = 64,
	};

	State state;
	std::string fen;
	std::string moves;
	int search_depth;
	Position position;
	PositionSearcher searcher;
};