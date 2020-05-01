#ifndef benchmark_h
#define benchmark_h

#include "evaluation.h"
#include "utils.h"
#include "parameters.h"
#include "position.h"

namespace Medusa
{
	using namespace Evaluation;

	void test_evaluation()
	{
		auto pos = PositionFromFen("5k1r/1Rp1r3/2n1pp2/2Pp4/p4P1p/b3BRPB/P1P4P/6K1 b - - 3 31");
		pos = PositionFromFen("rnbqkbnr/p3pppp/8/2p5/2N5/8/PPPPPP1R/R1BQKB1R w KQkq - 0 1");
		Parameters params;
		StaticScore(pos, params);
	}

	void test_infinite_score()
	{
		auto inf = Score::Infinite();
		auto mate = Score::Checkmate(1);
		auto cp = Score::Centipawns(100, 1);
		/*
		std::cout << (inf > mate) << (-inf < mate) << (mate > cp) << (-mate < cp) \
				  << (inf > cp) << ((-inf) < cp) << ((-inf) < inf) << (inf > -inf)    \
				  << std::endl;
		*/
		std::cout << (inf > -inf) << std::endl;

	}

	void test_strange_pawn_capture()
	{
		// Bug: "5k1r/1Rp1r3/2n1pp2/2Pp4/p4P1p/b3BRPB/P1P4P/6K1 b - - 3 31" attempted h4a2? 
		// Needed to prevent taking if file is A or H
		auto pos = PositionFromFen("5k1r/1Rp1r3/2n1pp2/2Pp4/p4P1p/b3BRPB/P1P4P/6K1 b - - 3 31");
		auto moves = pos.LegalPawnMoves<Any>(pos.ToMove());
		for (auto m : moves)
		{
			std::cout << m << ": " << AsUci(m) << std::endl;
		}
		system("pause");
	}

	void test_legal_moves_king()
	{
		// Bug: one of the pawn attacks tables (e7) was incorrect.

		auto position = PositionFromFen("");
		position.ApplyUCI("e2e4");
		position.ApplyUCI("b8c6");
		position.ApplyUCI("d2d4");
		position.ApplyUCI("g8f6");
		position.ApplyUCI("b1c3");
		position.ApplyUCI("d7d5");
		position.ApplyUCI("e4d5");
		position.ApplyUCI("f6d5");
		position.ApplyUCI("g1f3");
		position.ApplyUCI("d5c3");
		position.ApplyUCI("b2c3");
		position.ApplyUCI("d8d5");
		position.ApplyUCI("c1e3");
		position.ApplyUCI("c8f5");
		position.ApplyUCI("f1d3");
		position.ApplyUCI("e8c8");
		position.ApplyUCI("d3f5");
		position.ApplyUCI("d5f5");
		position.ApplyUCI("d1d3");
		position.ApplyUCI("f5d3");
		position.ApplyUCI("c2d3");
		position.ApplyUCI("d8d7");
		position.ApplyUCI("c3c4");
		position.ApplyUCI("c6b4");
		position.ApplyUCI("e1d2");
		position.ApplyUCI("d7d8");
		position.ApplyUCI("h1b1");
		position.ApplyUCI("b4c6");
		position.ApplyUCI("a2a4");
		position.ApplyUCI("d8d7");
		position.ApplyUCI("a4a5");
		position.ApplyUCI("h8g8");
		position.ApplyUCI("a5a6");
		position.ApplyUCI("b7b6");
		position.ApplyUCI("d4d5");
		position.ApplyUCI("c6a5");
		position.ApplyUCI("f3d4");
		position.ApplyUCI("d7d8");
		position.ApplyUCI("d4c6");
		position.ApplyUCI("a5c6");
		position.ApplyUCI("d5c6");
		position.ApplyUCI("d8d6");
		position.ApplyUCI("b1b2");
		position.ApplyUCI("d6g6");
		position.ApplyUCI("b2b1");
		position.ApplyUCI("g6g2");
		position.ApplyUCI("b1b2");
		position.ApplyUCI("g8h8");
		position.ApplyUCI("a1b1");
		position.ApplyUCI("g2h2");
		position.ApplyUCI("b2b3");
		position.ApplyUCI("h8g8");
		position.ApplyUCI("b3b2");
		position.ApplyUCI("h2h3");
		position.ApplyUCI("b2b3");
		position.ApplyUCI("g8h8");
		position.ApplyUCI("b3b2");
		position.ApplyUCI("h3h4");
		position.ApplyUCI("b2b3");
		position.ApplyUCI("h8g8");
		position.ApplyUCI("b3b2");
		position.ApplyUCI("h4h5");
		position.ApplyUCI("b2b3");
		position.ApplyUCI("h5a5");
		position.ApplyUCI("b3b4");
		position.ApplyUCI("a5a2");
		position.ApplyUCI("d2c3");
		position.ApplyUCI("a2a3");
		position.ApplyUCI("b4b3");
		position.ApplyUCI("a3b3");
		position.ApplyUCI("b1b3");
		position.ApplyUCI("g8h8");
		position.ApplyUCI("c3d4");
		position.ApplyUCI("c8b8");
		position.ApplyUCI("e3f4");
		position.ApplyUCI("h8g8");
		position.ApplyUCI("d4e5");
		position.ApplyUCI("f7f6");
		position.ApplyUCI("e5e6");
		position.ApplyUCI("g7g5");
		position.ApplyUCI("e6d7");
		position.ApplyUCI("g5f4");
		position.ApplyUCI("f2f3");
		position.ApplyUCI("g8g3");
		position.ApplyUCI("d3d4");
		position.ApplyUCI("f8h6");
		position.ApplyUCI("d4d5");
		position.ApplyUCI("g3g1");
		position.ApplyUCI("c4c5");
		position.ApplyUCI("g1g5");
		position.ApplyUCI("c5b6");
		position.ApplyUCI("g5d5");

		auto moves = position.LegalMoves<Any>();
		for (auto m : moves)
		{
			std::cout << m << ": " << AsUci(m) << std::endl;
		}

		position.ApplyUCI("d7e7");
	}

	void test_game_phase()
	{
		// Openings
		const std::string openings[] = {
			"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
			"rnbqkb1r/ppp1pppp/5n2/3p4/3P4/5N2/PPP1PPPP/RNBQKB1R w KQkq - 2 3",
			"r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3",
			"r1bqk2r/2ppbppp/p1n2n2/1p2p3/4P3/1B3N2/PPPP1PPP/RNBQR1K1 b kq - 1 7",
			"rnbq1rk1/ppp2pbp/3p1np1/4p3/2PPP3/2N2N2/PP2BPPP/R1BQ1RK1 b - - 1 7",
			"rnbqk2r/pp1pppbp/5np1/2p5/2P5/5NP1/PP1PPPBP/RNBQ1RK1 b kq - 3 5",
		};

		// Middlegames
		const std::string middlegames[] = {
			"r1b2rk1/2qn1ppp/p2pp3/1p3Pb1/3NP3/2N2Q2/PPP4P/1K1R1B1R w - - 2 15",
			"r1b2rk1/2q1bppp/2np1n2/pp2p3/3PP3/1N3N1P/PPB2PP1/R1BQR1K1 w - - 0 15",
			"2k4r/ppprbppp/2n5/8/4RB2/2N5/PPP2PPP/R5K1 w - - 1 15"
		};

		// Endgames
		const std::string endgames[] = {
			"4k3/N1p2rp1/7p/1pr1p3/3n4/P3R2P/2p3PK/2R5 w - - 0 29",
			"8/8/8/1pp5/k1b5/8/3K4/8 b - - 1 58",
			"8/pp6/2p1b3/6P1/P2b2kP/1P6/2P5/3NK3 w - - 1 40"
		};

		// Check game phase
		for (const auto &fen : openings)
		{
			auto pos = PositionFromFen(fen);
			auto phase_white = CalcGamePhase<Evaluation::White>(pos);
			auto phase_black = CalcGamePhase<Evaluation::Black>(pos);
			std::cout << fen << std::endl;
			std::cout << phase_white << phase_black << std::endl;
		}

		// Check game phase
		for (const auto &fen : middlegames)
		{
			auto pos = PositionFromFen(fen);
			auto phase_white = CalcGamePhase<Evaluation::White>(pos);
			auto phase_black = CalcGamePhase<Evaluation::Black>(pos);
			std::cout << fen << std::endl;
			std::cout << phase_white << phase_black << std::endl;
		}

		// Check game phase
		for (const auto &fen : endgames)
		{
			auto pos = PositionFromFen(fen);
			auto phase_white = CalcGamePhase<Evaluation::White>(pos);
			auto phase_black = CalcGamePhase<Evaluation::Black>(pos);
			std::cout << fen << std::endl;
			std::cout << phase_white << phase_black << std::endl;
		}
	}

	void test_bitboard_arithmetic()
	{
		auto bb = Bitboard(2);
		auto bb2 = bb - 1;
		bb &= bb2;

		auto bb3 = Bitboard(127);
		bb3 &= (bb3 - 1);

		auto it = BitIterator<Bitboard>(bb3);
		
		std::cout << "test_bitboard_arithmetic()" << std::endl;
	}

	void test_promotion_capture()
	{
		auto pos = PositionFromFen("6r1/7P/p7/Pr6/4pPK1/2k1B3/5P2/8 w - - 0 52");
		auto legals = pos.LegalPawnMoves<Any>(1);
		for (auto m : legals)
		{
			std::cout << AsUci(m) << std::endl;
		}
	}

	void test_equality_operator()
	{
		std::array<Bitboard, 2> a{ Bitboard(0), Bitboard(1) };
		std::array<Bitboard, 2> b{ Bitboard(0), Bitboard(2) };
		if(a==b)
			std::cout << "(a == b)" << std::endl;
		else
			std::cout << "(a != b)" << std::endl;
	}

	void test_moves_sanity()
	{
		Position new_pos = PositionFromFen("");
		new_pos.ApplyUCI("b1a3");
		new_pos.ApplyUCI("b8c6");
		new_pos.ApplyUCI("a3c4");
		new_pos.ApplyUCI("d7d5");
		new_pos.ApplyUCI("d2d4");
		new_pos.ApplyUCI("c6d4");

	}

	void benchmarks()
	{
		auto new_pos = PositionFromFen("");
		new_pos.ApplyUCI("d2d4");
		new_pos.ApplyUCI("b8c6");
		new_pos.ApplyUCI("b1a3");
		auto moves = new_pos.LegalMoves<Any>();
		for (auto move : moves)
		{
			std::cout << AsUci(move) << std::endl;
		}
		new_pos.PrettyPrint();
	}
}

#endif

