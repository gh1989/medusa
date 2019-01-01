#pragma once

class Score
{
public:
	Score() 
	{
		centipawns_for = 0;
		unavoidable_mate = false;
		mate_in = 0;
		depth = 0;
	}

	static Score Checkmate(size_t number)
	{
		Score score;
		score.centipawns_for = 0;
		score.unavoidable_mate = true;
		score.mate_in = number;
		score.depth = number;
		return score;
	}

	static Score Centipawns(int centipawns, int depth)
	{
		Score score;
		score.centipawns_for = centipawns;
		score.unavoidable_mate = false;
		score.mate_in = 0;
		score.depth;
		return score;
	}

	bool operator==(const Score& rhs) const
	{
		return (rhs.unavoidable_mate == unavoidable_mate) && (rhs.mate_in == mate_in) && (rhs.centipawns_for == centipawns_for);
	}


	// LHS < RHS if LHS is a worse score (faster mate or lower centipawns)
	bool operator<(const Score& rhs) const
	{
		bool lhs_mating = mate_in > 0;
		bool rhs_mating = rhs.mate_in > 0;

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
		if ( abs(centipawns_for - rhs.centipawns_for) < tolerance )
			return (depth < rhs.depth);

		// No checkmates going on, just compare the centipawns.
		return (centipawns_for < rhs.centipawns_for);
	}

	bool operator!=(const Score& rhs) const
	{
		return !(*this == rhs);
	}

	bool operator>(const Score& rhs) const
	{
		return ((*this) != rhs) && !(*this < rhs);
	}
	
	bool operator>=(const Score& rhs) const
	{
		return ((*this) > rhs) || (*this) == rhs;
	}

	bool operator<=(const Score& rhs) const
	{
		return ((*this) < rhs) || (*this) == rhs;
	}

	bool is_mate() const
	{
		return unavoidable_mate;
	}

	int get_mate_in() const {
		return mate_in/2+1;
	}

	double get_centipawns() const {
		return centipawns_for;
	}

	int get_depth() const {
		return depth;
	}

	Score operator-() const {
		Score negative_score;
		negative_score.mate_in = -mate_in;
		negative_score.centipawns_for = -centipawns_for;
		negative_score.unavoidable_mate = unavoidable_mate;
		negative_score.depth = depth;
		return negative_score;
	}

	Score operator-(int centipawns) const {
		return *this + (-centipawns);
	}

	Score operator+(int centipawns) const {
		Score new_score;
		new_score.mate_in = mate_in;
		new_score.centipawns_for = centipawns_for + centipawns;
		new_score.unavoidable_mate = unavoidable_mate;
		new_score.depth = depth;
		return new_score;
	}

private:

	bool unavoidable_mate;

	int mate_in;

	int centipawns_for;

	int depth;
};