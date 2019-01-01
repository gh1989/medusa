#include "uci.h"
#include <iostream>

int main()
{
	Bitboard::populate();
	UciCommand uci;
	std::string command;
	bool quit = false;
	while (!quit)
	{
		std::getline(std::cin, command);
		uci.process(command);
	}

	system("pause");
	return 0;
}