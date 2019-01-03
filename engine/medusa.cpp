#include "uci.h"
#include <iostream>

#include "move_tiny.h"
#include "position.h"


int main()
{
	Bitboard::populate();

	UciCommand uci;
	std::string command;
	bool quit = false;
	while (!quit)
	{
		std::getline(std::cin, command);
		if (command == "quit")
			break;
		uci.process(command);
	}
	return 0;
}