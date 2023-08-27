#include "ProgramObjectSet.h"

#include "compiler/builder/ClassBuilder.h"
#include "compiler/ast/Statements.h"
#include "compiler/ast/Values.h"

#include "Overloaded.h"

#include <set>
#include <list>
#include <vector>

using namespace comp;
using namespace comp::ast;

struct ElementReferences
{
	std::vector<std::shared_ptr<Function>> functions;
	std::vector<std::shared_ptr<Class>> classes;

	inline void addFunction(std::shared_ptr<Function> fn)
	{
		if(std::find(functions.begin(), functions.end(), fn) == functions.end())
		{
			functions.push_back(fn);
		}
	}

	inline void addClass(std::shared_ptr<Class> c)
	{
		if(std::find(classes.begin(), classes.end(), c) == classes.end())
		{
			classes.push_back(c);
		}
	}

	inline void operator()(const Call& c) { addFunction(c.fn); }
	inline void operator()(const Create& c) { if(c.type) addClass(c.type); }
	inline void operator()(const Dereference& c) { addClass(c.field.type); }
};

template<class C>
static inline void walkExpressionTree(const RValue& val, C&& c)
{
	val.accept(overloaded{
		[&](const Ternary& v){
			walkExpressionTree(*v.condition, c);
			walkExpressionTree(*v.then, c);
			walkExpressionTree(*v.otherwise, c);
		},
		[&](const Binary& v)
		{
			walkExpressionTree(*v.first, c);
			walkExpressionTree(*v.second, c);
		},
		[&](const Unary& v)
		{
			walkExpressionTree(*v.arg, c);
		},
		[&](const Call& v){
			c(v);
			std::for_each(v.args.begin(), v.args.end(), [&](const auto& v){ walkExpressionTree(*v, c); });
		},
		[&](const Create& v){
			c(v);
		},
		[&](const Dereference& v){
			c(v);
			walkExpressionTree(*v.object, c);
		},
		[&](const Set& v)
		{
			walkExpressionTree(*v.target, c);
			walkExpressionTree(*v.value, c);
		},
		[&](const Local& v){},
		[&](const Argument& v){},
		[&](const Literal& v){},
		[&](const auto& o){},
	});
}

template<class C>
static inline void walkBlockTree(const Statement& stmt, C&& c)
{
	stmt.accept(overloaded{
		[&](const Block& v)
		{
			for(const auto& stmt: v.stmts)
			{
				walkBlockTree(*stmt, c);
			}
		},
		[&](const Conditional& v)
		{
			walkExpressionTree(*v.condition, c);
			walkBlockTree(*v.then, c);
			walkBlockTree(*v.otherwise, c);
		},
		[&](const Loop& v) { walkBlockTree(*v.body, c); },
		[&](const ExpressionStatement& v) { walkExpressionTree(*v.val, c); },
		[&](const Declaration& v) { walkExpressionTree(*v.initializer, c); },
		[&](const Return& v) { std::for_each(v.value.begin(), v.value.end(), [&](const auto &w){walkExpressionTree(*w, c); }); },
		[&](const Continue& v) {},
		[&](const Break& v) {},
	});
}

static inline auto gatherReferences(std::shared_ptr<Function> entryPoint)
{
	std::vector<std::pair<std::shared_ptr<Function>, ElementReferences>> ret;
	std::list<std::shared_ptr<Function>> toDo;
	std::set<std::shared_ptr<Function>> done;

	toDo.push_back(entryPoint);

	while(!toDo.empty())
	{
		auto current = toDo.front();
		toDo.pop_front();
		done.insert(current);

		ElementReferences refs;
		walkBlockTree(*current->body, refs);
		ret.push_back(std::make_pair(current, refs));

		for(auto r: refs.functions)
		{
			if(done.find(r) == done.end())
			{
				toDo.push_back(r);
			}
		}
	}

	return ret;
}

static inline ElementReferences summarizeReferences(const std::vector<std::pair<std::shared_ptr<Function>, ElementReferences>> &deps)
{
	ElementReferences ret;

	for(const std::pair<std::shared_ptr<Function>, ElementReferences>& d: deps)
	{
		ret.addFunction(d.first);
		std::for_each(d.second.functions.begin(), d.second.functions.end(), [&](const auto& v){ret.addFunction(v);});
		std::for_each(d.second.classes.begin(), d.second.classes.end(), [&](const auto& v){ret.addClass(v);});
	}

	return ret;
}

static inline std::shared_ptr<Class> gatherStaticFields(std::vector<std::shared_ptr<Class>> classes)
{
	auto b = ClassBuilder::make();

	for(const auto &c: classes)
	{
		for(const auto &s: c->staticTypes)
		{
			b.addField(s);
		}
	}

	return b.data;
}

ProgramObjectSet ProgramObjectSet::shakeTree(std::shared_ptr<Function> entryPoint)
{
	auto allRefs = summarizeReferences(gatherReferences(entryPoint));

	ProgramObjectSet ret;
	ret.functions = allRefs.functions;

	ret.classes.push_back(gatherStaticFields(allRefs.classes));
	std::copy(allRefs.classes.begin(), allRefs.classes.end(), std::back_inserter(ret.classes));

	return ret;
}

size_t ProgramObjectSet::getClassIndex(const Class* cl) const
{
	auto it = std::find_if(classes.begin(), classes.end(), [cl](const auto v){return cl == v.get();});
	assert(it != classes.end());
	return it - classes.begin();
}

size_t ProgramObjectSet::getFunctionIndex(const Function* fn) const
{
	auto it = std::find_if(functions.begin(), functions.end(), [fn](const auto v){return fn == v.get();});
	assert(it != functions.end());
	return it - functions.begin();
}
