#ifndef benchmark_h
#define benchmark_h

#include "evaluation.h"
#include "utils.h"
#include "parameters.h"
#include "position.h"

namespace medusa
{
	using namespace eval;

	void test_evaluation()
	{
		auto pos = position_from_fen("5k1r/1Rp1r3/2n1pp2/2Pp4/p4P1p/b3BRPB/P1P4P/6K1 b - - 3 31");
		pos = position_from_fen("rnbqkbnr/p3pppp/8/2p5/2N5/8/PPPPPP1R/R1BQKB1R w KQkq - 0 1");
		Parameters params;
		static_score(pos, params);
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
		auto pos = position_from_fen("5k1r/1Rp1r3/2n1pp2/2Pp4/p4P1p/b3BRPB/P1P4P/6K1 b - - 3 31");
		auto moves = pos.legal_pawn_moves<Any>(pos.colour_to_move());
		for (auto m : moves)
		{
			std::cout << m << ": " << as_uci(m) << std::endl;
		}
		system("pause");
	}

	void test_legal_moves_king()
	{
		// Bug: one of the pawn attacks tables (e7) was incorrect.

		auto position = position_from_fen("");
		position.apply_uci("e2e4");
		position.apply_uci("b8c6");
		position.apply_uci("d2d4");
		position.apply_uci("g8f6");
		position.apply_uci("b1c3");
		position.apply_uci("d7d5");
		position.apply_uci("e4d5");
		position.apply_uci("f6d5");
		position.apply_uci("g1f3");
		position.apply_uci("d5c3");
		position.apply_uci("b2c3");
		position.apply_uci("d8d5");
		position.apply_uci("c1e3");
		position.apply_uci("c8f5");
		position.apply_uci("f1d3");
		position.apply_uci("e8c8");
		position.apply_uci("d3f5");
		position.apply_uci("d5f5");
		position.apply_uci("d1d3");
		position.apply_uci("f5d3");
		position.apply_uci("c2d3");
		position.apply_uci("d8d7");
		position.apply_uci("c3c4");
		position.apply_uci("c6b4");
		position.apply_uci("e1d2");
		position.apply_uci("d7d8");
		position.apply_uci("h1b1");
		position.apply_uci("b4c6");
		position.apply_uci("a2a4");
		position.apply_uci("d8d7");
		position.apply_uci("a4a5");
		position.apply_uci("h8g8");
		position.apply_uci("a5a6");
		position.apply_uci("b7b6");
		position.apply_uci("d4d5");
		position.apply_uci("c6a5");
		position.apply_uci("f3d4");
		position.apply_uci("d7d8");
		position.apply_uci("d4c6");
		position.apply_uci("a5c6");
		position.apply_uci("d5c6");
		position.apply_uci("d8d6");
		position.apply_uci("b1b2");
		position.apply_uci("d6g6");
		position.apply_uci("b2b1");
		position.apply_uci("g6g2");
		position.apply_uci("b1b2");
		position.apply_uci("g8h8");
		position.apply_uci("a1b1");
		position.apply_uci("g2h2");
		position.apply_uci("b2b3");
		position.apply_uci("h8g8");
		position.apply_uci("b3b2");
		position.apply_uci("h2h3");
		position.apply_uci("b2b3");
		position.apply_uci("g8h8");
		position.apply_uci("b3b2");
		position.apply_uci("h3h4");
		position.apply_uci("b2b3");
		position.apply_uci("h8g8");
		position.apply_uci("b3b2");
		position.apply_uci("h4h5");
		position.apply_uci("b2b3");
		position.apply_uci("h5a5");
		position.apply_uci("b3b4");
		position.apply_uci("a5a2");
		position.apply_uci("d2c3");
		position.apply_uci("a2a3");
		position.apply_uci("b4b3");
		position.apply_uci("a3b3");
		position.apply_uci("b1b3");
		position.apply_uci("g8h8");
		position.apply_uci("c3d4");
		position.apply_uci("c8b8");
		position.apply_uci("e3f4");
		position.apply_uci("h8g8");
		position.apply_uci("d4e5");
		position.apply_uci("f7f6");
		position.apply_uci("e5e6");
		position.apply_uci("g7g5");
		position.apply_uci("e6d7");
		position.apply_uci("g5f4");
		position.apply_uci("f2f3");
		position.apply_uci("g8g3");
		position.apply_uci("d3d4");
		position.apply_uci("f8h6");
		position.apply_uci("d4d5");
		position.apply_uci("g3g1");
		position.apply_uci("c4c5");
		position.apply_uci("g1g5");
		position.apply_uci("c5b6");
		position.apply_uci("g5d5");

		auto moves = position.legal_moves<Any>();
		for (auto m : moves)
		{
			std::cout << m << ": " << as_uci(m) << std::endl;
		}

		position.apply_uci("d7e7");
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
			auto pos = position_from_fen(fen);
			auto phase_white = game_phase<eval::White>(pos);
			auto phase_black = game_phase<eval::Black>(pos);
			std::cout << fen << std::endl;
			std::cout << phase_white << phase_black << std::endl;
		}

		// Check game phase
		for (const auto &fen : middlegames)
		{
			auto pos = position_from_fen(fen);
			auto phase_white = game_phase<eval::White>(pos);
			auto phase_black = game_phase<eval::Black>(pos);
			std::cout << fen << std::endl;
			std::cout << phase_white << phase_black << std::endl;
		}

		// Check game phase
		for (const auto &fen : endgames)
		{
			auto pos = position_from_fen(fen);
			auto phase_white = game_phase<eval::White>(pos);
			auto phase_black = game_phase<eval::Black>(pos);
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
		auto pos = position_from_fen("6r1/7P/p7/Pr6/4pPK1/2k1B3/5P2/8 w - - 0 52");
		auto legals = pos.legal_pawn_moves<Any>(1);
		for (auto m : legals)
		{
			std::cout << as_uci(m) << std::endl;
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
		Position new_pos = position_from_fen("");
		new_pos.apply_uci("b1a3");
		new_pos.apply_uci("b8c6");
		new_pos.apply_uci("a3c4");
		new_pos.apply_uci("d7d5");
		new_pos.apply_uci("d2d4");
		new_pos.apply_uci("c6d4");

	}

	void benchmarks()
	{
		auto new_pos = position_from_fen("");
		new_pos.apply_uci("d2d4");
		new_pos.apply_uci("b8c6");
		new_pos.apply_uci("b1a3");
		auto moves = new_pos.legal_moves<Any>();
		for (auto move : moves)
		{
			std::cout << as_uci(move) << std::endl;
		}
		new_pos.pp();
	}
}

#endif

