#ifndef COMPILER_REFERENCEEXRACTION_H_
#define COMPILER_REFERENCEEXRACTION_H_

#include "Overloaded.h"

#include "builder/ClassBuilder.h"

#include "model/Class.h"
#include "model/Block.h"
#include "model/Function.h"
#include "model/Statement.h"
#include "model/Conditional.h"

#include <set>
#include <map>

namespace comp {

struct ElementReferences
{
	std::set<std::shared_ptr<Function>> functions;
	std::set<std::shared_ptr<Class>> classes;

	inline void operator()(const Call& c) { functions.insert(c.fn); }
	inline void operator()(const Create& c) { if(c.type) classes.insert(c.type); }
	inline void operator()(const Dereference& c) { classes.insert(c.field.type); }
};

template<class C>
static inline void walkBlockTree(const Statement& stmt, C&& c)
{
	stmt.accept(overloaded{
		[&c](const Block& v)
		{
			for(const auto& stmt: v.stmts)
			{
				walkBlockTree(*stmt, c);
			}
		},
		[&c](const Conditional& v)
		{
			c(v);
			walkBlockTree(*v.then, c);
			if(v.otherwise) walkBlockTree(*v.otherwise, c);
		},
		[&c](const auto& o){ c(o); },
	});
}

template<class C>
static inline void walkExpressionTree(const RValue& val, C&& c)
{
	val.accept(overloaded{
		[&c](const Ternary& v){
			walkExpressionTree(*v.condition, c);
			walkExpressionTree(*v.then, c);
			walkExpressionTree(*v.otherwise, c);
		},
		[&c](const Binary& v)
		{
			walkExpressionTree(*v.first, c);
			walkExpressionTree(*v.second, c);
		},
		[&c](const Unary& v)
		{
			walkExpressionTree(*v.arg, c);
		},
		[&c](const Call& v){
			c(v);
			std::for_each(v.args.begin(), v.args.end(), [&c](const auto& v){ walkExpressionTree(*v, c); });
		},
		[&c](const Create& v){
			c(v);
		},
		[&c](const Dereference& v){
			c(v);
			walkExpressionTree(*v.object, c);
		},
		[&c](const Local& v){},
		[&c](const Argument& v){},
		[&c](const Literal& v){},
		[&c](const auto& o){},
	});
}

static inline ElementReferences extractReferences(std::shared_ptr<Function> fn)
{
	ElementReferences ret;

	walkBlockTree(*fn->body, overloaded
	{
		[&ret](const ExpressionStatement& v)
		{
			walkExpressionTree(*v.val, ret);
		},
		[&ret](const Conditional& v)
		{
			walkExpressionTree(*v.condition, ret);
		},
		[&ret](const Declaration& v)
		{
			walkExpressionTree(*v.initializer, ret);
		},
		[&ret](const Return& v)
		{
			std::for_each(v.value.begin(), v.value.end(), [&ret](const auto &w){walkExpressionTree(*w, ret); });
		},
		[&ret](const Set& v)
		{
			walkExpressionTree(*v.target, ret);
			walkExpressionTree(*v.value, ret);
		},
		[](const auto& o) {}
	});

	return ret;
}

static inline std::map<std::shared_ptr<Function>, ElementReferences> gatherReferences(std::shared_ptr<Function> entryPoint)
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

static inline ElementReferences summarizeReferences(const std::map<std::shared_ptr<Function>, ElementReferences> &deps)
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
static inline std::map<E, size_t> assignIndices(const std::set<E> &set, const E& first)
{
	std::map<E, size_t> ret;
	int i = 0;

	ret.insert({first, i++});

	for(const auto &e: set)
	{
		ret.insert({e, i++});
	}

	return ret;
}

static inline std::shared_ptr<Class> gatherStaticFields(std::set<std::shared_ptr<Class>> classes)
{
	auto c = ClassBuilder::make();

	for(const auto &c: classes)
	{
		(void)c; // TODO
	}

	return c.data;
}

} // namespace comp

#endif /* COMPILER_REFERENCEEXRACTION_H_ */
