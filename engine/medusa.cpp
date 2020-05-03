#include <iostream>
#include <string>

#include "benchmark.h"
#include "utils/logging.h"
#include "uci.h"

using namespace Medusa;

int main()
{
	//test_bizarre_knight_sac();

	// Logging
	auto now = std::chrono::system_clock::now();
	auto filename = "medusa_" + FormatTime(now) + ".txt";
	Logging::Get().SetFilename(filename);

	// UCI loop
	UciLoop uci_loop;
	uci_loop.RunLoop();
}