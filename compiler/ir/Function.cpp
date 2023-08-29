#include "Function.h"
#include "Terminations.h"

#include "Overloaded.h"
#include "assert.h"

#include <map>
#include <set>
#include <sstream>
#include <algorithm>

using namespace comp;
using namespace comp::ir;

static inline const std::map<Conditional::Condition, std::string> conditionOp =
{
	{Conditional::Condition::Eq, "=="},
	{Conditional::Condition::Ne, "!="},
	{Conditional::Condition::LtI, "<"},
	{Conditional::Condition::GtI, ">"},
	{Conditional::Condition::LeI, "<="},
	{Conditional::Condition::GeI, ">="},
	{Conditional::Condition::LtU, "<"},
	{Conditional::Condition::GtU, ">"},
	{Conditional::Condition::LeU, "<="},
	{Conditional::Condition::GeU, ">="},
	{Conditional::Condition::LtF, "<"},
	{Conditional::Condition::GtF, ">"},
	{Conditional::Condition::LeF, "<="},
	{Conditional::Condition::GeF, ">="},
};

static inline Conditional::Condition inverse(Conditional::Condition c)
{
	switch(c)
	{
		default:
		case Conditional::Condition::Eq: return Conditional::Condition::Ne;
		case Conditional::Condition::Ne: return Conditional::Condition::Eq;
		case Conditional::Condition::LtI: return Conditional::Condition::GeI;
		case Conditional::Condition::GtI: return Conditional::Condition::LeI;
		case Conditional::Condition::LeI: return Conditional::Condition::GtI;
		case Conditional::Condition::GeI: return Conditional::Condition::LtI;
		case Conditional::Condition::LtU: return Conditional::Condition::GeU;
		case Conditional::Condition::GtU: return Conditional::Condition::LeU;
		case Conditional::Condition::LeU: return Conditional::Condition::GtU;
		case Conditional::Condition::GeU: return Conditional::Condition::LtU;
		case Conditional::Condition::LtF: return Conditional::Condition::GeF;
		case Conditional::Condition::GtF: return Conditional::Condition::LeF;
		case Conditional::Condition::LeF: return Conditional::Condition::GtF;
		case Conditional::Condition::GeF: return Conditional::Condition::LtF;
	}
}

std::string ir::Function::dump(ast::ProgramObjectSet& gi) const
{
	std::map<std::shared_ptr<BasicBlock>, size_t> blocks;
	auto getIdx = [&](auto i)
	{
		auto it = blocks.find(i);
		return (it != blocks.end()) ? it->second : blocks.insert({i, blocks.size()}).first->second;
	};

	std::stringstream ss;
	BasicBlock::DumpContext dc;

	ss << "digraph G {" << std::endl;


	ss << "\tentry -> 0[label=\"fun(";

	const char* sep = "";
	for(const auto &a: args) {
		ss << sep << dc.nameOf(a);
		sep = ", ";
	}

	ss << ")\"]" << std::endl;

	traverse([&](std::shared_ptr<BasicBlock> bb)
	{
		const auto idx =  getIdx(bb);
		ss << "\t" << idx << "[shape=rect label=\"" << bb->dump(gi, dc) << "\"]" << std::endl;

		bb->termination->accept(overloaded
		{
			[&](const Always& v) {
				ss << "\t" << idx << " -> " << getIdx(v.continuation) << std::endl;
			},
			[&](const Conditional& v)
			{
				ss << "\t" << idx << " -> " << getIdx(v.then) << "[label=\"" << dc.nameOf(v.first) << " " << conditionOp.find(v.condition)->second << " " << dc.nameOf(v.second) << "\"]"<< std::endl;
				ss << "\t" << idx << " -> " << getIdx(v.otherwise) << "[label=\"" << dc.nameOf(v.first) << " " << conditionOp.find(inverse(v.condition))->second << " " << dc.nameOf(v.second) << "\"]"<< std::endl;
			},
			[&](const Leave& v)
			{
				ss << "\t" << idx << " -> exit" << "[label=\"" << "\"]"<< std::endl;
			},
		});
	});

	ss << "}" << std::endl;

	const auto ret = ss.str();
	return ret;
}

void Function::traverse(std::function<void(std::shared_ptr<BasicBlock>)> c) const
{
	std::set<std::shared_ptr<BasicBlock>> toDo{entry}, done;

	while(!toDo.empty())
	{
		const auto current = *toDo.begin();
		toDo.erase(current);
		if(done.insert(current).second)
		{
			c(current);

			current->termination->accept(overloaded
			{
				[&](const Leave &v){},
				[&](const Always &v)
				{
					toDo.insert(v.continuation);
				},
				[&](const Conditional &v)
				{
					toDo.insert(v.then);
					toDo.insert(v.otherwise);
				},
			});
		}
	}
}
