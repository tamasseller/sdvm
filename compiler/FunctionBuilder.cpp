#include "FunctionBuilder.h"

#include "model/Block.h"

#include "model/Statement.h"

#include <set>
#include <map>
#include <list>

using namespace comp;

struct ElementReferences
{
	std::set<std::shared_ptr<Function>> functions;
	std::set<std::shared_ptr<Class>> classes;
};

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

template<class C>
void walkBlockTree(const Block& block, C&& c)
{
	for(const auto& stmt: block.stmts)
	{
		stmt->accept(overloaded{
			[&c](const Block& b){ walkBlockTree(b, c); },
			[&c](const auto& o){ c(o); },
		});
	}
}

ElementReferences extractReferences(std::shared_ptr<Function> fn)
{
	ElementReferences ret;

	walkBlockTree(*fn->body, overloaded
	{
		[&ret](const Call& c) {ret.functions.insert(c.fn);}, // TODO also walk expressions
		[](const auto& o) {}
	});

	return ret;
}

std::map<std::shared_ptr<Function>, ElementReferences> gatherReferences(std::shared_ptr<Function> entryPoint)
{
	std::set<std::shared_ptr<Function>> toDo{entryPoint};
	std::map<std::shared_ptr<Function>, ElementReferences> ret;

	while(!toDo.empty())
	{
		auto current = *toDo.begin();
		toDo.erase(toDo.begin());

		auto refs = extractReferences(current);
		ret.insert({current, refs});

		for(auto r: refs.functions)
		{
			if(!ret.count(r))
			{
				toDo.insert(r);
			}
		}
	}

	return ret;
}

ElementReferences summarizeReferences(const std::map<std::shared_ptr<Function>, ElementReferences> &deps)
{
	ElementReferences ret;

	for(const auto& d: deps)
	{
		ret.functions.insert(d.first);
		ret.functions.insert(d.second.functions.begin(), d.second.functions.end());
		ret.classes.insert(d.second.classes.begin(), d.second.classes.end());
	}

	return ret;
}

template<class E>
std::map<E, size_t> assignIndices(const std::set<E> &set, int i = 0)
{
	std::map<E, size_t> ret;

	for(const auto &e: set)
	{
		ret.insert({e, i++});
	}

	return ret;
}

prog::Program FunctionBuilder::compile() const
{
	prog::Program ret;

	auto allRefs = summarizeReferences(gatherReferences(data));
	auto fnIdxTable = assignIndices(allRefs.functions);
	auto classIdxTable = assignIndices(allRefs.classes);

	return ret;
}
