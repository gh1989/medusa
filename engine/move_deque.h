#ifndef kernel_h
#define kernel_h

#ifndef _SILENCE_PARALLEL_ALGORITHMS_EXPERIMENTAL_WARNING
#define _SILENCE_PARALLEL_ALGORITHMS_EXPERIMENTAL_WARNING
#endif

#include "position.h"

#include <algorithm>
#include <memory>
#include <vector>
#include <list>
#include <sstream>
#include <string>

class MoveDeque
{
public:
	MoveDeque() : move(0) {};
	MoveDeque(const MoveDeque&) = default;
	MoveDeque(MoveDeque&&) = default;
	MoveDeque& operator=(const MoveDeque&) = default;
	MoveDeque& operator=(MoveDeque&&) = default;
	~MoveDeque() = default;

	std::shared_ptr<MoveDeque> before;
	std::shared_ptr<MoveDeque> after;
	MoveTiny move;

	static void join(
		std::shared_ptr<MoveDeque> md, 
		std::shared_ptr<MoveDeque> md_after,
		MoveTiny move)
	{
		md_after->before = md;
		md->after = md_after;
		md->move  = move;
	}

	static std::shared_ptr<MoveDeque> end(std::shared_ptr<MoveDeque> md)
	{
		std::shared_ptr<MoveDeque> tmp = md;

		// Go forward.
		while (tmp->after != nullptr)
		{
			tmp = tmp->after;
		}

		return tmp;
	}

	static void apply(Position &pos, std::shared_ptr<MoveDeque> md)
	{
		std::shared_ptr<MoveDeque> tmp = md;

		// Go forward.
		while (tmp->after != nullptr)
		{
			pos.apply(tmp->move);
			tmp = tmp->after;
		}
	}

	static void unapply(Position &pos, std::shared_ptr<MoveDeque> md)
	{
		std::shared_ptr<MoveDeque> tmp = MoveDeque::end(md);
		
		// Go back.
		while (tmp->before != nullptr)
		{
			tmp = tmp->before;
			pos.unapply(tmp->move);
		};
	}

	static void print_line(std::shared_ptr<MoveDeque> md) 
	{
		std::cout << get_line(md) << std::endl;
	};

	static std::string get_line(std::shared_ptr<MoveDeque> md)
	{
		std::shared_ptr<MoveDeque> tmp = md;
		std::stringstream ss;

		while (tmp->before != nullptr)
			tmp = tmp->before;

		while (tmp->after != nullptr)
		{
			ss << as_uci(tmp->move) << " ";
			tmp = tmp->after;
		}
					
		return ss.str();
	};

	static size_t size(std::shared_ptr<MoveDeque> md)
	{
		std::shared_ptr<MoveDeque> tmp = md;
		size_t sz = 0;

		while (tmp->before != nullptr)
			tmp = tmp->before;

		while (tmp->after != nullptr)
		{ 
			tmp = tmp->after;
			sz++; 
		}
			
		return sz;
	}
	   
};

#endif
