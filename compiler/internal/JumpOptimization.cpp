#include "Compiler.h"

#include "compiler/ir/Function.h"
#include "compiler/ir/BasicBlock.h"
#include "compiler/ir/Terminations.h"

#include "Overloaded.h"

#include <set>
#include <algorithm>

using namespace comp;
using namespace comp::ast;

bool Compiler::removeEmptyBasicBlocks(std::shared_ptr<ir::Function> f)
{
	bool ret = false;

	f->traverse([&](std::shared_ptr<ir::BasicBlock> bb)
	{
		bb->termination->accept(overloaded
		{
			[&](const ir::Leave&){},
			[&](const ir::Always &t)
			{
				if(t.continuation->code.empty())
				{
					bb->termination = t.continuation->termination;
					ret = true;
				}
			},
			[&](const ir::Conditional& t)
			{
				if(auto a = std::dynamic_pointer_cast<ir::Always>(t.then->termination); a && t.then->code.empty())
				{
					bb->termination = std::make_shared<ir::Conditional>(t.condition, t.first, t.second, a->continuation, t.otherwise);
					ret = true;
				}

				if(auto a = std::dynamic_pointer_cast<ir::Always>(t.otherwise->termination); a && t.otherwise->code.empty())
				{
					bb->termination = std::make_shared<ir::Conditional>(t.condition, t.first, t.second, t.then, a->continuation);
					ret = true;
				}
			},
		});
	});

	return ret;
}

bool Compiler::mergeBasicBlocks(std::shared_ptr<ir::Function> f)
{
	std::set<std::shared_ptr<ir::BasicBlock>> ok, notOk;

	auto consider = [&](std::shared_ptr<ir::BasicBlock> bb)
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

	f->traverse([&](std::shared_ptr<ir::BasicBlock> bb)
	{
		bb->termination->accept(overloaded
		{
			[&](const ir::Leave&){},
			[&](const ir::Always &t) { consider(t.continuation); },
			[&](const ir::Conditional& t) { consider(t.then); consider(t.otherwise); },
		});
	});

	if(!ok.empty())
	{
		bool ret = false;
		f->traverse([&](std::shared_ptr<ir::BasicBlock> bb)
		{
			bb->termination->accept(overloaded
			{
				[&](const ir::Conditional&) {},
				[&](const ir::Leave&) {},
				[&](const ir::Always &t)
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
