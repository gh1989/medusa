#include <chrono>
#include <ctime>
#include <functional>

#include "parameters.h"
#include "search.h"
#include "move.h"

#include "utils/logging.h"

namespace Medusa {

bool Search::searching_flag = true;

size_t Search::Perft(Position &position, size_t max_depth)
{
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
	threads.emplace_back([this, pos, max_depth]() {
		auto cpos = pos;
		SearchRoot(cpos, max_depth);
	});
}

std::shared_ptr<Variation> Search::SearchRoot(Position &pos, int max_depth)
{
	std::shared_ptr<Variation> md(new Variation());
	nodes_searched = 0;

	Score alpha = Score::Checkmate(-1);
	Score beta  = Score::Checkmate(1);
	
	// Now do a search on this aperture size of score +/- delta
	md = std::shared_ptr<Variation>(new Variation());
	auto currentScore = this->_Search(pos, md, alpha, beta, 1);
	PositionHistory::Clear();

	for (int d = 2; d <= max_depth; d++)
	{
		auto tmp_md = std::shared_ptr<Variation>(new Variation());
		this->_Search(pos, tmp_md, alpha, beta, d);
		PositionHistory::Clear();
		md = tmp_md;
	}

	return md;
}

Score Search::_Search(
	Position &pos,
	std::shared_ptr<Variation> md,
	Score alpha,
	Score beta,
	int max_depth)
{
	Score score;
	if (pos.ToMove().IsBlack())
		score = -this->_Search(pos, md, -beta, -alpha, max_depth, 0);
	else
		score = this->_Search(pos, md, alpha, beta, max_depth, 0);
	return score;
}

Score Search::_Search(
	Position &pos,
	std::shared_ptr<Variation> moves_all,
	Score alpha,
	Score beta,
	int max_depth,
	size_t plies_from_root)
{
	if (max_depth < -60)
		throw;

	auto now = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - search_start_time);
	
	if (elapsed.count() >= search_time_limit)
	{
		searching_flag = false;
	}

	if (!searching_flag)
		return alpha;
	bool qsearch = max_depth <= 0;

	MoveSelector move_selector(pos, !qsearch, params);

	if (qsearch)
	{
		int centipawns = Evaluation::StaticScore(pos, params);
		auto c = pos.ToMove();
		auto stand_pat = Score::Centipawns(c*centipawns, plies_from_root);
		if (stand_pat >= beta)
			return stand_pat;
		if (alpha < stand_pat)
			alpha = stand_pat;
	}

	// Draw
	if (pos.GetFiftyCounter() > 49 || pos.ThreeMoveRepetition())
		return Score::Centipawns(0, plies_from_root);

	if (!move_selector.Any())
	{
		// Checkmate?
		if (pos.IsCheckmate())
			return -Score::Checkmate(plies_from_root);

		// Draw?
		bool search_all = max_depth > 0;
		if (!pos.IsInCheck() && search_all)
			return Score::Centipawns(0, plies_from_root);

		// Position evaluation to be maximized or minimized.
		Parameters params;
		double centipawns = Evaluation::StaticScore(pos, params);

		// Since we are always the maximizer we are always technically
		// white.
		auto c = pos.ToMove();
		return Score::Centipawns(c*centipawns, plies_from_root);
	}
	
	// Lowest possible score from the view of the maximiser.
	Score best_score = -Score::Infinite();
	Move best_move;
	Score score;
	std::vector<PvInfo> infos;
	auto moves = move_selector.GetMoves();
	nodes_searched += moves.size();
	for (auto pair : moves)
	{
		if (!searching_flag)
			break;
		auto move = pair.second;
		std::shared_ptr<Variation> moves_after(new Variation());
		pos.Apply(move);
		score = -this->_Search(pos, moves_after, -beta, -alpha, max_depth - 1, plies_from_root + 1);
		pos.Unapply(move);

		// Update the move variation if we have an improvement.
		if (score > best_score)
		{
			best_score = score;
			Join(moves_all, moves_after, move);
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
		if (best_score > beta)
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

	//return alpha;
	return best_score;
}

MoveSelector::MoveSelector(Position& position, bool include_quiet, const Parameters &params_)
{
	params = params_;
	Position pos = position;
	auto unordered_moves = pos.LegalMoves<Medusa::Any>();
	auto colour = position.ToMove();

	int i = 0;
	for (auto &move : unordered_moves)
	{
		auto pc = position.GetAttacker(move);
		auto from_square = GetFrom(move);
		int promise = PAWN - pc;
		bool is_capture = position.MoveIsCapture(move);
		position.Apply(move);
		bool is_check = position.IsInCheck();

		if (!include_quiet && !(is_check || is_capture))
		{
			position.Unapply(move);
			continue;
		}

		if (is_check)
		{
			if (!position.AnyLegalMove())
			{
				position.Unapply(move);
				moves.clear();
				moves.insert({ 0, move });
				break;
			}
			else
				promise += 100;
		}

		// Needs to be a good reason to move a piece twice.
		if (position.LastMoved( pc, from_square) )
			promise -= 10;

		position.Unapply(move);
		moves.insert({ -promise, move });
	};
}
}
