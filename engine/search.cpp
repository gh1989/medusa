#include "position_searcher.h"
#include "move_deque.h"
#include "eval.h"
#include "move_selector.h"

bool PositionSearcher::searching_flag = true;

std::shared_ptr<MoveDeque> PositionSearcher::search(Position &pos, int max_depth)
{
	std::shared_ptr<MoveDeque> md(new MoveDeque());
	
	Score alpha = Score::Checkmate(-1);
	Score beta  = Score::Checkmate(1);
	Score score = this->search(pos, md, alpha, beta, max_depth);
	std::cout << "bestmove " << md->move.as_uci() << std::endl;
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
		throw;
	}

	if (!PositionSearcher::searching_flag)
		return alpha;
	bool qsearch = max_depth <= 0;
	MoveSelector move_selector(pos, !qsearch);

	if (!move_selector.any() 
		||  pos.get_fifty_counter() > 49 
	    ||  pos.three_move_repetition() )
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
		// To get out of searching
		if (!PositionSearcher::searching_flag)
			break;

		auto move = pair.second;

		std::shared_ptr<MoveDeque> moves_after(new MoveDeque());
		move.apply(pos);
		score = -this->search(pos, moves_after, -beta, -alpha, max_depth - 1, plies_from_root + 1);
		move.unapply(pos);

		// Update the move deque if we have an improvement.
		if (score > best_score)
		{
			MoveDeque::join(moves_all, moves_after, move);
			if (plies_from_root == 0)
				PositionSearcher::print_info(moves_all, score, max_depth);
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

		if (max_depth < -50)
		{
			std::cout << pos.get_piece_bitboard(Colour::BLACK, Piece::ROOK).get_bit_number() << std::endl;
			std::cout << pos.get_piece_bitboard(Colour::WHITE, Piece::ROOK).get_bit_number() << std::endl;
			pos.pp();
		}
	}

	return alpha;
}

void PositionSearcher::print_info(
	const std::shared_ptr<MoveDeque>& md, 
	Score score, 
	size_t depth )
{
	auto line = MoveDeque::get_line(md);
	std::cout << "info pv " << line;
	if (!score.is_mate())
		std::cout << "score cp " << score.get_centipawns() << std::endl;
	else
		std::cout << "score mate " << score.get_mate_in() << std::endl;
}