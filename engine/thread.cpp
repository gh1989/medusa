#include "search.h"
#include "thread.h"

namespace Medusa
{
	void Thread::StartThread(
		Position &pos,
		int max_depth,
		BestMoveInfo::Callback bestmovecallback,
		PvInfo::Callback infocallback)
	{
		Search search;
		search.SetSearchingFlag(true);
		search.SetInfoCallback(infocallback);
		auto searching_lambda = [&search, pos, max_depth, bestmovecallback]
		{
			auto cpos = pos;
			search.SearchRoot(cpos, max_depth);
			bestmovecallback(search.GetBestMoveInfo());
		};
		threads.emplace_back(searching_lambda);
	}
	/*		
	void Thread::Start(const Position &pos, int max_depth)
	{
		Search search;
		auto _searching_lambda = [this, pos, max_depth]() {
			auto cpos = pos;
			SearchRoot(cpos, max_depth);
		};
		threads.emplace_back(_searching_lambda);
	}*/
}