#include "types.h"

namespace medusa
{
	const Colour Colour::WHITE = Colour(1);
	const Colour Colour::BLACK = Colour(-1);

	Score::Score()
	{
		centipawns_for = 0;
		unavoidable_mate = false;
		mate_in = 0;
		depth = 0;
	}

	Score Score::Checkmate(size_t number)
	{
		Score score;
		score.centipawns_for = 0;
		score.unavoidable_mate = true;
		score.mate_in = number;
		score.depth = number;
		return score;
	}

	Score Score::Centipawns(int centipawns, int depth)
	{
		Score score;
		score.centipawns_for = centipawns;
		score.unavoidable_mate = false;
		score.mate_in = 0;
		score.depth;
		return score;
	}

	Score Score::Infinite()
	{
		Score score;
		score.infinite = 1;
		return score;
	}

	bool Score::operator==(const Score& rhs) const
	{
		return (infinite == rhs.infinite) && (rhs.unavoidable_mate == unavoidable_mate) && (rhs.mate_in == mate_in) && (rhs.centipawns_for == centipawns_for);
	}
	
	// LHS < RHS if LHS is a worse score (faster mate or lower centipawns)
	bool Score::operator<(const Score& rhs) const
	{
		bool lhs_mating = mate_in > 0;
		bool rhs_mating = rhs.mate_in > 0;

		// RHS = inf so LHS < RHS
		if (rhs.infinite > 0)
			return true;

		// LHS = -inf so LHS < RHS
		if (infinite < 0)
			return true;

		// RHS = -inf so LHS > RHS
		if (rhs.infinite < 0)
			return false;

		// LHS = inf so LHS > RHS
		if (infinite > 0)
			return false;

		// If one is unavoidable mate and the other is not it matters whether being mated.
		if (rhs.unavoidable_mate && !unavoidable_mate)
		{
			// Is RHS getting mated or mating? No centipawn score is worse than getting mated.
			return rhs_mating;
		}

		if (!rhs.unavoidable_mate && unavoidable_mate)
		{
			// Is the RHS not getting mated but LHS is? Then LHS must be < 0 to be less than RHS.
			return !lhs_mating;
		}

		// Both are unavoidable mates, the direction and moves to mate makes a difference.
		if (rhs.unavoidable_mate && unavoidable_mate)
		{
			// LHS is being mated, RHS is mating. LHS is worse.
			if (!lhs_mating && rhs_mating)
				return true;

			// LHS is mating, RHS is being mated. LHS is better.
			if (lhs_mating && !rhs_mating)
				return false;

			// Both are mating, the worst one mates longer.
			if (lhs_mating && rhs_mating)
				return (mate_in > rhs.mate_in);

			// Both are being mated, the worst one mates faster (less negative)
			if (!lhs_mating && !rhs_mating)
				return (mate_in > rhs.mate_in);
		}

		// We want the deepest to be the best.
		double tolerance = 1e-5;
		if (abs(centipawns_for - rhs.centipawns_for) < tolerance)
			return (depth < rhs.depth);

		// No checkmates going on, just compare the centipawns.
		return (centipawns_for < rhs.centipawns_for);
	}

	bool Score::operator!=(const Score& rhs) const
	{
		return !(*this == rhs);
	}

	bool Score::operator>(const Score& rhs) const
	{
		return ((*this) != rhs) && !(*this < rhs);
	}

	bool Score::operator>=(const Score& rhs) const
	{
		return ((*this) > rhs) || (*this) == rhs;
	}

	bool Score::operator<=(const Score& rhs) const
	{
		return ((*this) < rhs) || (*this) == rhs;
	}

	bool Score::is_mate() const
	{
		return unavoidable_mate;
	}

	int Score::get_mate_in() const {
		return mate_in / 2 + 1;
	}

	double Score::get_centipawns() const {
		return centipawns_for;
	}

	int Score::get_depth() const {
		return depth;
	}

	// Negative, infinite is negative infinite.
	Score Score::operator-() const {
		Score negative_score;
		negative_score.mate_in = -mate_in;
		negative_score.centipawns_for = -centipawns_for;
		negative_score.unavoidable_mate = unavoidable_mate;
		negative_score.depth = depth;
		negative_score.infinite = -infinite;
		return negative_score;
	}

	// Subtract centipawns (infinite is still infinite)
	Score Score::operator-(int centipawns) const {
		return *this + (-centipawns);
	}

	// Add centipawns, does not affect infinite.
	Score Score::operator+(int centipawns) const {
		Score new_score;
		new_score.mate_in = mate_in;
		new_score.centipawns_for = centipawns_for + centipawns;
		new_score.unavoidable_mate = unavoidable_mate;
		new_score.depth = depth;
		new_score.infinite = infinite;
		return new_score;
	}
};