#include <chrono>
#include <ctime>
#include <functional>

#include "search.h"
#include "move.h"

#include "utils/logging.h"

namespace Medusa {

bool Search::searching_flag = true;

size_t Search::Perft(Position &position, size_t max_depth)
{
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
	threads.emplace_back([this, pos, max_depth]() {
		auto cpos = pos;
		SearchRoot(cpos, max_depth);
	});
}

std::shared_ptr<Variation> Search::SearchRoot(Position &pos, int max_depth)
{
	std::shared_ptr<Variation> md(new Variation());
	nodes_searched = 0;

	// least to be expected: mated in one
	Score alpha = Score::Checkmate(-1);

	// most which can be achieved: to mate in one
	Score beta  = Score::Checkmate(1);

	md = std::shared_ptr<Variation>(new Variation());
	auto currentScore = this->_Search(pos, md, alpha, beta, max_depth);
	PositionHistory::Clear();
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
	/*
	if (max_depth < -60)
		throw;
	auto now = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - search_start_time);
	if (elapsed.count() >= search_time_limit)
		searching_flag = false;
	if (!searching_flag)
		return alpha;
	*/

	bool qsearch = max_depth <= 0;
	MoveSelector move_selector(pos, !qsearch);
	
	if (qsearch)
	{
		// Maximizer score.
		int centipawns = Evaluation::StaticScore(pos);
		if (pos.ToMove().IsBlack())
			centipawns *= -1;

		auto stand_pat = Score::Centipawns(centipawns, plies_from_root);

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
		{
			int mate_in = plies_from_root;
			if (pos.ToMove().IsBlack()) // White's turn and mated, so black mate in one.
				mate_in *= -1;
			return Score::Checkmate(mate_in);
		}

		// Draw?
		bool search_all = max_depth > 0;
		if (!pos.IsInCheck() && search_all)
			return Score::Centipawns(0, plies_from_root);

		// Position evaluation to be maximized or minimized. If we are black and the
		// static score of the position of the node is very negative, that is a high
		// score for the node, * -1...
		double centipawns = Evaluation::StaticScore(pos);
		if (pos.ToMove().IsBlack())
			centipawns *= -1;

		return Score::Centipawns(centipawns, plies_from_root);
	}
	// Lowest possible score from the view of the maximiser.
	Score score;
	std::vector<PvInfo> infos;
	auto moves = move_selector.GetMoves();
	nodes_searched += moves.size();

	Score value = -Score::Infinite();
	for (auto pair : moves)
	{
		//if (!searching_flag)
		//	break;
		auto move = pair.second;

		int __dbg = false;
		if (move == CreateMove(e5, c6))
			int __dbg = true;
		
		// --- apply ---
		pos.Apply(move);
		std::shared_ptr<Variation> moves_after(new Variation());
		score = -this->_Search(
			pos, moves_after, -beta, -alpha, max_depth - 1, plies_from_root + 1);
		// --- unapply ---
		pos.Unapply(move);

		// Update this node value
		if ( score > value)
		{		
			value = score;
		}

		// Update alpha
		if (score > alpha)
		{
			alpha = score;
			Join(moves_all, moves_after, move);
			//PrintLine(moves_all);

			// Update the PV info each time this happens.
			PvInfo pv_info;
			pv_info.depth = max_depth;
			pv_info.score = score;
			pv_info.pv = moves_all;
			pv_info.nodes = nodes_searched;
			infos.push_back(pv_info);
		}

		// Cut off
		if (alpha >= beta)
			break;

		if (qsearch && (score >= beta))
			return beta;
	}


	// Update best move if we made it back to root in time...  //	if (searching_flag)
	if (plies_from_root == 0)
	{
		Mutex::Lock lock(counters_mutex);
		best_move_info.best_move = moves_all->move;
		best_move_info.score = value; // Which will be alpha
		best_move_info.depth = max_depth;
#ifndef _DEBUG
		info_callback(infos);
#endif // !DEBUG_

	}

	return alpha;
}

MoveSelector::MoveSelector(Position& position, bool include_quiet)
{
	Position pos = position;

	if (pos.GetPlies() == 2)
	{
		int j = 0;
	}

	auto unordered_moves = pos.LegalMoves<Medusa::Any>();
	auto colour = position.ToMove();

	int i = 0;

	// What the player is forked, or checked as part of a combination. The escape is not going to be 
	// a non-quiet move necessary, e.g. a block or a king move, but it can still be part of a forced
	// combo. (*) ...
	// Unfortunately this causes problems with the search.
	//auto was_in_check = position.IsInCheck();

	for (auto &move : unordered_moves)
	{
		auto pc = position.GetAttacker(move);
		auto from_square = GetFrom(move);
		int promise = PAWN - pc;
		bool is_capture = position.MoveIsCapture(move);
		position.Apply(move);
		bool is_check = position.IsInCheck();

		if (!include_quiet && !(is_check || is_capture)  /*&& !was_in_check*/)
		{
			position.Unapply(move);
			continue;
		}

		// (*) ... this seems to be a temporary fix. Prevents pointless perpetuals
		if (is_capture)
			promise += 1000;

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
			promise -= 20;

		position.Unapply(move);
		moves.insert({ -promise, move });
	};
}
}
