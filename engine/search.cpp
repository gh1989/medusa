#include <chrono>
#include <ctime>
#include <functional>

#include "search.h"
#include "evaluation.h"
#include "move.h"

namespace medusa {

bool Search::searching_flag = true;

size_t Search::perft(Position &position, size_t max_depth)
{
	if (max_depth == 0)
		return 1;

	size_t nodes = 0;
	auto pseudo_legals = position.pseudo_legal_moves();
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

std::shared_ptr<MoveDeque> Search::search_root(Position &pos, int max_depth)
{
	std::shared_ptr<MoveDeque> md(new MoveDeque());
	nodes_searched = 0;
	search_start_time = std::chrono::system_clock::now();
	Score alpha = Score::Checkmate(-1);
	Score beta  = Score::Checkmate(1);
	this->search(pos, md, alpha, beta, max_depth);
	PositionHistory::Clear();
	return md;
}

Score Search::search(
	Position &pos,
	std::shared_ptr<MoveDeque> md, 
	Score alpha,
	Score beta,
	int max_depth )
{
	Score score;
	if (pos.colour_to_move().is_black())
		score = -this->search(pos, md, -beta, -alpha, max_depth, 0);
	else
		score = this->search(pos, md, alpha, beta, max_depth, 0);
	std::cout << "bestmove " << as_uci(md->move) << std::endl;
	return score;
}

Score Search::search(
	Position &pos, 
	std::shared_ptr<MoveDeque> moves_all,
	Score alpha, 
	Score beta, 
	int max_depth, 
	size_t plies_from_root )
{
	if (max_depth < -60)
		throw;

	if (!Search::searching_flag)
		return alpha;
	bool qsearch = max_depth <= 0;

	MoveSelector move_selector(pos, !qsearch);

	if (qsearch)
	{
		int centipawns = eval::static_score(pos);
		auto c = pos.colour_to_move();
		auto stand_pat = Score::Centipawns(c*centipawns, plies_from_root);
		if (stand_pat >= beta)
			return beta;
		if (alpha < stand_pat)
			alpha = stand_pat;
	}

	if (!move_selector.any() 
		|| pos.get_fifty_counter() > 49 
	    || pos.three_move_repetition()
		)
	{
		// Checkmate?
		if (pos.is_checkmate())
			return -Score::Checkmate(plies_from_root);

		// Draw?
		bool search_all = max_depth > 0;
		if (!pos.in_check() && search_all)
			return Score::Centipawns(0, plies_from_root);

		// Position evaluation to be maximized or minimized.
		double centipawns = eval::static_score(pos);

		// Since we are always the maximizer we are always technically
		// white.
		auto c = pos.colour_to_move();
		return Score::Centipawns(c*centipawns, plies_from_root);
	}
	
	// The worst possible score in which we are getting mated in a single move.
	Score best_score = Score::Checkmate(-1);
	Score score;

	for (auto pair : move_selector.get_moves())
	{
		// To get out of searching
		if (!Search::searching_flag)
			break;

		nodes_searched++;
		auto move = pair.second;
		std::shared_ptr<MoveDeque> moves_after(new MoveDeque());
		pos.apply(move);
		score = -this->search(pos, moves_after, -beta, -alpha, max_depth - 1, plies_from_root + 1);
		pos.unapply(move);

		// Update the move deque if we have an improvement.
		if (score > best_score)
		{
			MoveDeque::join(moves_all, moves_after, move);
			if (plies_from_root == 0)
			{
				auto end = std::chrono::system_clock::now();
				auto elapsed  = std::chrono::duration_cast<std::chrono::milliseconds>(end - search_start_time);
				Search::print_info(moves_all, score, max_depth, nodes_searched, elapsed.count());
			}
		}

		if (score > alpha)
			alpha = score;
		if (score > best_score)
			best_score = score;
		if (alpha >= beta)
			break;

	}

	return alpha;
}

void Search::print_info(
	const std::shared_ptr<MoveDeque>& md,
	Score score,
	size_t depth,
	size_t nodes,
	long long time)
{
	double time_seconds = double(time) / 1000;
	size_t nps = nodes / time_seconds;
	auto line = MoveDeque::get_line(md);
	std::cout << "info time " << time << " ";
	std::cout << "nodes " << nodes << " ";
	std::cout << "nps " << nps << " ";
	std::cout << "pv " << line;

	if (!score.is_mate())
		std::cout << "score cp " << score.get_centipawns() << std::endl;
	else
		std::cout << "score mate " << score.get_mate_in() << std::endl;
}

MoveSelector::MoveSelector(Position& position, bool include_quiet)
{
	auto unordered_moves = position.legal_moves();
	auto colour = position.colour_to_move();

	int i = 0;
	for (auto &move : unordered_moves)
	{
		// Move promise score
		int promise = PAWN - position.attacker(move);

		// Exit the loop if actually we are quiescent and this
		// is a quiet move.
		bool is_capture = position.is_capture(move);

		// === Apply ===
		position.apply(move);

		bool is_check = position.in_check();

		if (!include_quiet && !(is_check || is_capture))
		{
			// Skip! Remember to unapply the move.
			position.unapply(move);
			continue;
		}

		if (is_check)
		{
			// Checkmate...
			if (!position.any_legal_move())
			{
				position.unapply(move);
				moves.clear();
				moves.insert({ 0, move });
				break;
			}
			else
				promise += 1000;
		}

		// === Unapply ===
		position.unapply(move);
		moves.insert({ -promise, move });
	};
}

int MoveSelector::see(Position &pos, Move move)
{
	int value = 0;

	// Get smallest attacker capture
	auto next_moves = pos.legal_moves();
	auto remove_condition = [pos](Move m) { return pos.was_capture(m); };
	auto to_remove = std::remove_if(next_moves.begin(), next_moves.end(), remove_condition);
	next_moves.erase(to_remove, next_moves.end());

	bool capture_exists = next_moves.size();
	if (!capture_exists)
	{
		return value;
	}

	// Get the smallest attacker
	auto next_move = next_moves.back();
	int smallest_attacker = 100000;
	for (auto m : next_moves)
	{
		int attacker = values[pos.attacker(next_move)];
		if (attacker <= smallest_attacker)
		{
			smallest_attacker = attacker;
			next_move = m;
		}
	}

	// === Apply ====
	pos.apply(next_move);
	int capture_value = values[pos.captured(next_move)] - see(pos, next_move);
	pos.unapply(next_move);
	// === unapply ====

	value = capture_value > 0 ? capture_value : 0;
	return value;
}

}
