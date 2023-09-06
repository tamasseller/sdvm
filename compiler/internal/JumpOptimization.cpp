#include "Compiler.h"

#include "compiler/ir/Function.h"
#include "compiler/ir/BasicBlock.h"
#include "compiler/ir/Terminations.h"

#include "Overloaded.h"

#include <set>
#include <algorithm>

using namespace comp;
using namespace comp::ir;

static inline bool canReach(std::shared_ptr<BasicBlock> start, std::shared_ptr<BasicBlock> destination)
{
	bool ret = false;

	BasicBlock::traverse(start, [&](const std::shared_ptr<BasicBlock> &bb)
	{
		if(bb == destination)
		{
			ret = true;
		}
	});

	return ret;
}

bool Compiler::removeEmptyBasicBlocks(std::shared_ptr<Function> f)
{
	bool ret = false;

	f->traverse([&](std::shared_ptr<BasicBlock> bb)
	{
		bb->termination->accept(overloaded
		{
			[&](const Leave&){},
			[&](const Always &t)
			{
				if(t.continuation->code.empty())
				{
					bb->termination = t.continuation->termination;
					ret = true;
				}
			},
			[&](const Conditional& t)
			{
				if(auto a = std::dynamic_pointer_cast<Always>(t.then->termination); a && t.then->code.empty() && !canReach(a->continuation, bb))
				{
					bb->termination = std::make_shared<Conditional>(t.condition, t.first, t.second, a->continuation, t.otherwise);
					ret = true;
				}
				else if(auto a = std::dynamic_pointer_cast<Always>(t.otherwise->termination); a && t.otherwise->code.empty() && !canReach(a->continuation, bb))
				{
					bb->termination = std::make_shared<Conditional>(t.condition, t.first, t.second, t.then, a->continuation);
					ret = true;
				}
			},
		});
	});

	return ret;
}

bool Compiler::mergeBasicBlocks(std::shared_ptr<Function> f)
{
	std::set<std::shared_ptr<BasicBlock>> ok, notOk;

	auto consider = [&](std::shared_ptr<BasicBlock> bb)
	{
		if(notOk.find(bb) == notOk.end())
		{
			auto it = ok.find(bb);
			if(it == ok.end())
			{
				ok.insert(bb);
			}
			else
			{
				ok.erase(bb);
				notOk.insert(bb);
			}
		}
	};

	f->traverse([&](std::shared_ptr<BasicBlock> bb)
	{
		bb->termination->accept(overloaded
		{
			[&](const Leave&){},
			[&](const Always &t) { consider(t.continuation); },
			[&](const Conditional& t) { consider(t.then); consider(t.otherwise); },
		});
	});

	if(!ok.empty())
	{
		bool ret = false;
		f->traverse([&](std::shared_ptr<BasicBlock> bb)
		{
			bb->termination->accept(overloaded
			{
				[&](const Conditional&) {},
				[&](const Leave&) {},
				[&](const Always &t)
				{
					if(ok.find(t.continuation) != ok.end())
					{
						std::copy(t.continuation->code.begin(), t.continuation->code.end(), std::back_inserter(bb->code));
						bb->termination = t.continuation->termination;
						ret = true;
					}

					consider(t.continuation);
				},
			});
		});

		return ret;
	}

	return false;
}
