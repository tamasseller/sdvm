#include "Tacify.h"

#include "compiler/ast/Values.h"
#include "compiler/ast/Statements.h"

#include "Overloaded.h"

using namespace comp;
using namespace comp::ast;

// proto register
struct Preg
{
	const ValueType type;
	inline Preg(ValueType type): type(type) {}
};

struct BasicBlockBuilder
{
	std::vector<std::shared_ptr<BasicBlock>> bbs;

	BasicBlockBuilder() {
		bbs.push_back(std::make_shared<BasicBlock>());
	}

	void add(std::shared_ptr<Statement> stmt) {
		bbs.back()->code->stmts.push_back(stmt);
	}

	std::shared_ptr<Local> addTemporary(std::shared_ptr<const RValue> val) {
		auto ret = std::make_shared<Local>(val->getType());
		bbs.back()->code->stmts.push_back(std::make_shared<Declaration>(ret, val));
		return ret;
	}

	std::pair<std::shared_ptr<BasicBlock>, std::shared_ptr<BasicBlock>> close()
	{
		auto old = bbs.back();
		bbs.push_back(std::make_shared<BasicBlock>());
		return {old, bbs.back()};
	}
};

static inline std::shared_ptr<const RValue> visitExpression(BasicBlockBuilder& bbb, std::shared_ptr<const RValue> val)
{
	std::shared_ptr<const RValue> ret;

	val->accept(overloaded
	{
		[&](const Local& v) { ret = v.shared_from_this(); },
		[&](const Global& v) { ret = v.shared_from_this(); },
		[&](const Argument& v) { ret = v.shared_from_this(); },
		[&](const Dereference& v) {
			ret = bbb.addTemporary(std::make_shared<Dereference>(visitExpression(bbb, v.object), v.field));
		},
		[&](const Unary& v) { ret = bbb.addTemporary(v.shared_from_this());},
		[&](const Create& v) { ret = bbb.addTemporary(v.shared_from_this());},
		[&](const Literal& v) { ret = bbb.addTemporary(v.shared_from_this()); },
		[&](const Set& v)
		{
			ret = visitExpression(bbb, v.value);

			auto target = v.target;
			target->accept(overloaded{
				[&](const Dereference& d){ target = std::make_shared<Dereference>(visitExpression(bbb, d.object), d.field); },
				[&](const auto& o){}
			});

			bbb.add(std::make_shared<ExpressionStatement>(std::make_shared<Set>(target, ret)));
		},
		[&](const Call& v)
		{
			std::vector<std::shared_ptr<const RValue>> args;
			std::transform(v.args.begin(), v.args.end(), std::back_inserter(args), [&](const auto& v){ return visitExpression(bbb, v); });
			auto call = std::make_shared<Call>(v.fn, std::move(args));
			if(call->fn->ret.size())
			{
				ret = bbb.addTemporary(call);
			}
			else
			{
				bbb.add(std::make_shared<ExpressionStatement>(call));
			}
		},
		[&](const Binary& v)
		{
			switch(v.op)
			{
				case Binary::Operation::And:
					// TODO eliminate with jumpy-jumpy
				case Binary::Operation::Or:
					// TODO  eliminate with jumpy-jumpy
					break;
				default:
				{
					ret = bbb.addTemporary(std::make_shared<Binary>(v.op, visitExpression(bbb, v.first), visitExpression(bbb, v.second)));
				}
			}
		},
		[&](const Ternary& v) {
			auto t = std::make_shared<Local>(v.getType());
			ret = t;

			auto decisionInput = visitExpression(bbb, v.condition);
			auto ifThenPoint = bbb.close();
			bbb.add(std::make_shared<ExpressionStatement>(std::make_shared<Set>(t, visitExpression(bbb, v.then))));
			auto thenElsePoint = bbb.close();
			bbb.add(std::make_shared<ExpressionStatement>(std::make_shared<Set>(t, visitExpression(bbb, v.otherwise))));
			auto endifPoint = bbb.close();

			ifThenPoint.first->decisionInput = decisionInput;
			ifThenPoint.first->then = thenElsePoint.first;
			ifThenPoint.first->otherwise= thenElsePoint.second;
			thenElsePoint.first->then = endifPoint.second;
			thenElsePoint.second->then = endifPoint.second;
		},
	});

	return ret;
}

static inline void visitStatement(BasicBlockBuilder& bbb, std::shared_ptr<Statement> stmt)
{
	stmt->accept(overloaded
	{
		[&](const ExpressionStatement& v){
			visitExpression(bbb, v.val);
		},
		[&](const Block& v){
			for(const auto& s: v.stmts)
			{
				visitStatement(bbb, s);
			}
		},
		[&](const Declaration& v){
			bbb.add(std::make_shared<Declaration>(v.local, visitExpression(bbb, v.initializer)));
		},
		[&](const Return& v){
			std::vector<std::shared_ptr<const RValue>> retvals;
			std::transform(v.value.begin(), v.value.end(), std::back_inserter(retvals), [&](const auto& v){ return visitExpression(bbb, v);});
			bbb.add(std::make_shared<Return>(std::move(retvals)));
		},
		[&](const Conditional& v){}, // TODO eliminate with jumpy-jumpy
		[&](const Continue& v){}, // TODO eliminate with jumpy-jumpy
		[&](const Break& v){}, // TODO eliminate with jumpy-jumpy
		[&](const Loop& v){}, // TODO eliminate with jumpy-jumpy
	});
}

std::vector<std::shared_ptr<BasicBlock>> comp::tacify(std::shared_ptr<Function> f)
{
	BasicBlockBuilder bbb;
	visitStatement(bbb, f->body);
	return bbb.bbs;
}
