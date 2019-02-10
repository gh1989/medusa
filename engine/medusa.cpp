#include <iostream>

#include "benchmark.h"
#include "uci.h"

int main()
{
	medusa::UciCommand uci;
	std::string command;
	bool quit = false;
	while (!quit)
	{
		std::getline(std::cin, command);
		if (command == "quit")
			break;
		if (command == "benchmark")
			medusa::test_moves();
		uci.process(command);
	}

	return 0;
}