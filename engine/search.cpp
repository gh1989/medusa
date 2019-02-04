#include "position_searcher.h"
#include "move_deque.h"
#include "eval.h"
#include "move_selector.h"

#include <chrono>
#include <ctime>
#include <functional>

bool PositionSearcher::searching_flag = true;

size_t PositionSearcher::perft(Position &position, size_t max_depth)
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

void _search_root(PositionSearcher *searcher, Position pos, int max_depth)
{
	searcher->search_root(pos, max_depth);
}

void PositionSearcher::stop()
{
	if (thread.joinable())
		thread.detach();
}

void PositionSearcher::search_thread(const Position &pos, int max_depth)
{
	if(thread.joinable())
		thread.detach();

	Position copy_pos = pos;
	thread = std::thread(_search_root, this, std::ref(copy_pos), std::ref(max_depth) );
}

std::shared_ptr<MoveDeque> PositionSearcher::search_root(Position &pos, int max_depth)
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

Score PositionSearcher::search(
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

Score PositionSearcher::search(
	Position &pos, 
	std::shared_ptr<MoveDeque> moves_all,
	Score alpha, 
	Score beta, 
	int max_depth, 
	size_t plies_from_root )
{
	if (max_depth < -60)
	{
		PositionSearcher::searching_flag = false;

		std::cout << pos.get_piece_bitboard(Colour::BLACK, Piece::ROOK).get_bit_number() << std::endl;
		std::cout << pos.get_piece_bitboard(Colour::WHITE, Piece::ROOK).get_bit_number() << std::endl;
		pos.pp();
	}

	if (!PositionSearcher::searching_flag)
		return alpha;
	bool qsearch = max_depth <= 0;

	MoveSelector move_selector(pos, !qsearch);

	if (qsearch)
	{
		int centipawns = Eval::static_score(pos);
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
		double centipawns = Eval::static_score(pos);

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
		nodes_searched++;

		// To get out of searching
		if (!PositionSearcher::searching_flag)
			break;

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
				//nodes_searched = perft(pos, max_depth);
				auto elapsed  = std::chrono::duration_cast<std::chrono::milliseconds>(end - search_start_time);
				PositionSearcher::print_info(moves_all, score, max_depth, nodes_searched, elapsed.count());
			}
		}
		if (score > alpha)
		{
			alpha = score;
		}
		if (score > best_score)
		{
			best_score = score;
		}
		if (alpha >= beta)
			break;

	}

	return alpha;
}

void PositionSearcher::print_info(
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