#ifndef position_searcher_h
#define position_searcher_h

#include "position.h"
#include "move.h"
#include "utils.h"
#include "evaluation.h"

namespace Medusa 
{
	class Search
	{
	public:
		Score QSearch(
			Position &position,
			Score alpha,
			Score beta,
			std::shared_ptr<Variation> vrtn,
			int dfr);

		Score QSearchEitherColour(
			Position& position_,
			Score alpha,
			Score beta,
			std::shared_ptr<Variation> vrtn);

		std::shared_ptr<Variation> IterWindowSearch(
			Position &position_,
			int max_depth_
		);

		std::shared_ptr<Variation> SearchRoot(Position &pos, int max_depth);
		//void Start(const Position &pos, int max_depth);
		
		size_t GetNodesSearched() const { return 0; }

		BestMoveInfo GetBestMoveInfo() const {
			return best_move_info;
		}

		void SetInfoCallback(PvInfo::Callback info_callback_) {
			info_callback = info_callback_;
		}

		static void SetSearchingFlag(bool value) {
			Search::searching_flag = value;
		}

	private:
		static size_t Perft(Position &position, size_t depth);
		static bool searching_flag;
		std::shared_ptr<Variation> principal_variation;
		int max_depth;
		BestMoveInfo best_move_info;
		PvInfo::Callback info_callback;
	};
}

#endif