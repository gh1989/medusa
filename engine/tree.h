#pragma once

#include "types.h"

namespace medusa
{
	struct MoveNode
	{
		Move move;
		MoveNode *parent;
		std::vector<MoveNode> children;
	};

	struct Tree
	{
		MoveNode root;
	};
}