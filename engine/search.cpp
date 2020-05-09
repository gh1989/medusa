#include <chrono>
#include <ctime>
#include <functional>

#include "search.h"
#include "move.h"
#include "moveiter.h"

#include "utils/logging.h"

namespace Medusa {

bool Search::searching_flag = true;

size_t Search::Perft(Position &position, size_t max_depth)
{
	/// Counts the number of nodes, currently turned off, it probably 
	/// will only work well if we have a hash table of positions otherwise
	/// it greedily recounts all of the legal moves which takes up too much
	/// resource.
	return 0;
	if (max_depth == 0)
		return 1;
	size_t nodes = 0;
	auto pseudo_legals = position.PseudoLegalMoves<Any>();
	for (auto m : pseudo_legals)
	{
		position.Apply(m);
		if (!position.IsInCheck())
			nodes += Perft(position, max_depth - 1);
		position.Unapply(m);
	}
	return nodes;
}

void Search::Start(const Position &pos, int max_depth)
{
	/// Start the thread. At the moment this is kind of trivial.
	/// The thread cannot exit mid search right now.
	auto _searching_lambda = [this, pos, max_depth](){
		auto cpos = pos;
		SearchRoot(cpos, max_depth);
	};
	threads.emplace_back(_searching_lambda);
}

std::shared_ptr<Variation> Search::SearchRoot(Position &position_, int max_depth)
{
	std::shared_ptr<Variation> vrtn(new Variation());

	/// Start the search with alpha and beta at maximum bounds
	/// +/- infinite, which is higher scoring than mate in one,
	/// in case there is a mate in one at depth 0.
	auto alpha = -Score::Infinite();
	auto beta = Score::Infinite();

	/// Every level of the search is a maximizer of the score,
	/// which means that the logic is essentially that of scoring
	/// in favour of the white pieces. So negate the search root 
	/// node for the black pieces if they are the root.
	if (position_.ToMove().IsBlack()) {
		-QSearch(position_, -beta, -alpha, vrtn, 0);
	}
	else {
		QSearch(position_, alpha, beta, vrtn, 0);
	}

	// Need to clear the position history, used for 3 move repetition.
	PositionHistory::Clear();
	return vrtn;
}

Score Evaluate(Position &position, int dfr)
{
	/// Take the centipawn positional evaluation and
	/// negate it if it's blacks turn because the score
	/// is always from the view of white pieces but the
	/// recursive algorithm treats the current maximizer
	/// as the white pieces.
	auto cp = Evaluation::StaticScore(position);
	if (position.ToMove().IsBlack())
		cp *= -1;
	auto stateval = Score::Centipawns(cp, dfr);
	
	/// If there are no legal moves it is either checkmate
	/// or it is stalemate. Since we are the maximizer right now
	/// and in checkmate, it will seem as though we are in 
	/// checkmate position from black pieces, so always have
	/// checkmate in -dfr.
	if (!position.AnyLegalMove())
	{
		if (position.IsInCheck())
			stateval = -Score::Checkmate(dfr);
		else
			stateval = Score::Centipawns(0, dfr);
	}

	/// Include the 50 move rule and three move repetition here
	/// as it is done at the beginning of every node and will 
	/// accordingly update alpha.
	//	TOOD: This does not currently work actually,
	//	three repetition is defective.
	if (position.GetFiftyCounter() >= 50 || position.ThreeMoveRepetition())
		stateval = Score::Centipawns(0, dfr);

	return stateval;
}

Score Search::QSearch(
	Position &position, 
	Score alpha, 
	Score beta, 
	std::shared_ptr<Variation> vrtn, 
	int dfr)
{
	/// Take the current score of the position for standing pat. We do not 
	/// take pieces unfavourably. The score is cp, if this is greater than
	/// beta then the move which has just been applied gives us greater  
	/// than what we know we can have, thefore opponent will not play it. 
	/// If the cp is greater than alpha then we increase the minimum which 
	/// can be expected.
	if (dfr > 0)
	{
		auto spat = Evaluate(position, dfr);
		if (spat >= beta)
			return spat;
		if (alpha < spat)
			alpha = spat;
	}

	/// If the current alpha is checkmate then it must have 
	/// been updated by the static evaluation of the position 
	/// used for standing pat and to detect draws or checkmates. 
	/// So here we return alpha. Current context is checkmated, 
	/// it gets negated to the parent maximizer.
	if (alpha.IsMate())
		return alpha;

	/// Iterate through moves which are included in 
	/// qsearch. This will be evades from checks, checks 
	/// themselves and captures, think about including types 
	/// of attacks too.

	/*
	// Not yet implemented, just use the move selector for now.
	auto moveIterator = position.MoveIterator( CAPTURES | EVADES | CHECKS ); 
	// Not yet implemented, use move selector.
	for (auto move = moveIterator.begin(); move != moveIterator.end(); ++move)
	*/

	MoveSelector msel(position, dfr == 0);
	auto mvs = msel.GetMoves();	
	for(auto mv : mvs)
	{
		/// Apply the move and then search all of the the new position
		/// but this time maximize for the opposition. Do this by 
		/// swapping the alpha to negative beta, beta to negative alpha 
		/// and negating the whole result.
		position.Apply(mv.second);
		std::shared_ptr<Variation> vrtnMore(new Variation());
		auto score = -QSearch( position, -beta, -alpha, vrtnMore, dfr+1);
		position.Unapply(mv.second);

		/// Update best score. Alpha is the beta of this branch, 
		/// beta is the most possible to be scored, if we are
		/// larger than beta then must return beta. If the beta score.
		/// It has to be larger than or equal to alpha in updating because
		/// the bounds are mate in one, and if there is a mate in one, 
		/// we still need to the move to update.
		if (score > alpha)
		{
			alpha = score;
			Join(vrtn, vrtnMore, mv.second);
		}

		/// If alpha is larger than beta, then the least expected from
		/// this branch is currently greater than the most we can expect.
		/// this may not be possible if we start with infinite bounds
		/// but when approximating alpha and beta in a search it can 
		/// happen and should fail for a research.
		if (alpha >= beta)
			return beta;
		
	}

	/// Here update the best move at the root node. The score will 
	/// be the returned alpha from the recursive function call.
	if (dfr == 0)
	{
		//Mutex::Lock lock(counters_mutex);
		best_move_info.best_move = vrtn->move;
		best_move_info.score = alpha;
	}
		
	return alpha;
}


}
