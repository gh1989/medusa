#include <chrono>
#include <ctime>
#include <functional>

#include "parameters.h"
#include "search.h"
#include "move.h"

namespace medusa {

bool Search::searching_flag = true;

size_t Search::perft(Position &position, size_t max_depth)
{
	if (max_depth == 0)
		return 1;

	size_t nodes = 0;
	auto pseudo_legals = position.pseudo_legal_moves<Any>();
	for (auto m : pseudo_legals)
	{
		position.apply(m);
		if (!position.in_check())
			nodes += perft(position, max_depth - 1);
		position.unapply(m);
	}

	return nodes;
}

void Search::start(const Position &pos, int max_depth)
{
	threads.emplace_back([this, pos, max_depth]() {
		auto cpos = pos;
		search_root(cpos, max_depth);
	});
}

std::shared_ptr<Variation> Search::search_root(Position &pos, int max_depth)
{
	std::shared_ptr<Variation> md(new Variation());
	nodes_searched = 0;
	Score alpha = Score::Checkmate(-1);
	Score beta  = Score::Checkmate(1);
	this->search(pos, md, alpha, beta, max_depth);
	PositionHistory::Clear();
	return md;
}

Score Search::search(
	Position &pos,
	std::shared_ptr<Variation> md,
	Score alpha,
	Score beta,
	int max_depth)
{
	Score score;
	if (pos.colour_to_move().is_black())
		score = -this->search(pos, md, -beta, -alpha, max_depth, 0);
	else
		score = this->search(pos, md, alpha, beta, max_depth, 0);
	return score;
}

Score Search::search(
	Position &pos,
	std::shared_ptr<Variation> moves_all,
	Score alpha,
	Score beta,
	int max_depth,
	size_t plies_from_root)
{
	if (max_depth < -60)
		throw;

	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - search_start_time);
	if (elapsed.count() >= search_time_limit)
		searching_flag = false;

	if (!searching_flag)
		return alpha;
	bool qsearch = max_depth <= 0;

	MoveSelector move_selector(pos, !qsearch, params);

	if (qsearch)
	{
		int centipawns = eval::static_score(pos, params);
		auto c = pos.colour_to_move();
		auto stand_pat = Score::Centipawns(c*centipawns, plies_from_root);
		if (stand_pat >= beta)
			return beta;
		if (alpha < stand_pat)
			alpha = stand_pat;
	}

	// Draw
	if (pos.get_fifty_counter() > 49 || pos.three_move_repetition())
		return Score::Centipawns(0, plies_from_root);

	if (!move_selector.any())
	{
		// Checkmate?
		if (pos.is_checkmate())
			return -Score::Checkmate(plies_from_root);

		// Draw?
		bool search_all = max_depth > 0;
		if (!pos.in_check() && search_all)
			return Score::Centipawns(0, plies_from_root);

		// Position evaluation to be maximized or minimized.
		Parameters params;
		double centipawns = eval::static_score(pos, params);

		// Since we are always the maximizer we are always technically
		// white.
		auto c = pos.colour_to_move();
		return Score::Centipawns(c*centipawns, plies_from_root);
	}
	
	// Lowest possible score from the view of the maximiser.
	Score best_score = -Score::Infinite();
	Move best_move;
	Score score;
	std::vector<PvInfo> infos;
	auto moves = move_selector.get_moves();
	nodes_searched += moves.size();
	for (auto pair : moves)
	{
		if (!searching_flag)
			break;
		auto move = pair.second;
		std::shared_ptr<Variation> moves_after(new Variation());
		pos.apply(move);
		score = -this->search(pos, moves_after, -beta, -alpha, max_depth - 1, plies_from_root + 1);
		pos.unapply(move);

		// Update the move variation if we have an improvement.
		if (score > best_score)
		{
			best_score = score;
			join(moves_all, moves_after, move);
			if (plies_from_root == 0)
			{
				best_move = move;
				PvInfo pv_info;
				pv_info.depth = max_depth;
				pv_info.score = score;
				pv_info.pv = moves_all;
				pv_info.nodes = nodes_searched;
				infos.push_back(pv_info);

				if (!best_move_info.best_move)
				{
					best_move_info.best_move = best_move;
					best_move_info.score = best_score;
					best_move_info.depth = max_depth;
				}
			}
		}

		if (score > alpha)
			alpha = score;
		if (score > best_score)
			best_score = score;
		if (alpha >= beta)
			break;

	}
	
	// Update best move if we made it back to root in time.
	if (plies_from_root == 0)
	if (searching_flag)
	{
		Mutex::Lock lock(counters_mutex);
		best_move_info.best_move = best_move;
		best_move_info.score = best_score;
		best_move_info.depth = max_depth;
		info_callback(infos);
	}

	return alpha;
}

MoveSelector::MoveSelector(Position& position, bool include_quiet, const Parameters &params_)
{
	params = params_;
	auto unordered_moves = position.legal_moves<Any>();
	auto colour = position.colour_to_move();

	int i = 0;
	for (auto &move : unordered_moves)
	{
		auto pc = position.attacker(move);
		auto from_square = from(move);
		int promise = PAWN - pc;
		bool is_capture = position.is_capture(move);
		position.apply(move);
		bool is_check = position.in_check();

		if (!include_quiet && !(is_check || is_capture))
		{
			position.unapply(move);
			continue;
		}

		if (is_check)
		{
			if (!position.any_legal_move())
			{
				position.unapply(move);
				moves.clear();
				moves.insert({ 0, move });
				break;
			}
			else
				promise += 100;
		}

		// Needs to be a good reason to move a piece twice.
		if (position.last_moved( pc, from_square) )
			promise -= 10;

		position.unapply(move);
		moves.insert({ -promise, move });
	};
}
}
