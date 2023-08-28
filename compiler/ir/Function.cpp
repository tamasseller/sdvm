#include "Function.h"

#include "Operations.h"

#include "compiler/ast/Values.h"
#include "compiler/ast/Statements.h"

#include "Overloaded.h"
#include "assert.h"

#include <map>
#include <sstream>
#include <algorithm>

using namespace comp;
using namespace comp::ir;

struct Context
{
	std::vector<std::shared_ptr<BasicBlock>> bbs;
	std::map<std::shared_ptr<const ast::Local>, std::shared_ptr<Temporary>> locals;
	std::vector<std::shared_ptr<Temporary>> args;

	Context(size_t nArgs)
	{
		for(auto i = 0u; i < nArgs; i++)
		{
			args.push_back(std::make_shared<Temporary>());
		}
	}

	void addOp(std::shared_ptr<Operation> stmt) {
		bbs.back()->code.push_back(stmt);
	}

	template<class C>
	auto addOp(C&& c)
	{
		auto ret = std::make_shared<Temporary>();
		bbs.back()->code.push_back(c(ret));
		return ret;
	}

	std::pair<std::shared_ptr<BasicBlock>, std::shared_ptr<BasicBlock>> close()
	{
		auto old = bbs.back();
		bbs.push_back(std::make_shared<BasicBlock>());
		return {old, bbs.back()};
	}

	void addLocal(std::shared_ptr<const ast::Local> local, std::shared_ptr<Temporary> t)
	{
		const bool inserted = locals.insert({local, t}).second;
		assert(inserted);
	}

	std::shared_ptr<Temporary> getLocal(std::shared_ptr<const ast::Local> local)
	{
		const auto it = locals.find(local);
		assert(it != locals.end());
		return it->second;
	}

	std::shared_ptr<Temporary> arg(size_t idx)
	{
		assert(idx < args.size());
		return args[idx];
	}
};

inline auto mapUnaryOp(ast::Unary::Operation op)
{
	switch(op)
	{
		default:
		case ast::Unary::Operation::Neg: return Unary::Op::Neg;
		case ast::Unary::Operation::I2F: return Unary::Op::I2F;
		case ast::Unary::Operation::F2I: return Unary::Op::F2I;
		case ast::Unary::Operation::Not: return Unary::Op::Not;
	}
}

inline auto mapBinaryOp(ast::Binary::Operation op)
{
	switch(op)
	{
		default:
		case ast::Binary::Operation::AddF: return Binary::Op::AddF;
		case ast::Binary::Operation::AddI: return Binary::Op::AddI;
		case ast::Binary::Operation::MulI: return Binary::Op::MulI;
		case ast::Binary::Operation::SubI: return Binary::Op::SubI;
		case ast::Binary::Operation::DivI: return Binary::Op::DivI;
		case ast::Binary::Operation::Mod:  return Binary::Op::Mod;
		case ast::Binary::Operation::ShlI: return Binary::Op::ShlI;
		case ast::Binary::Operation::ShrI: return Binary::Op::ShrI;
		case ast::Binary::Operation::ShrU: return Binary::Op::ShrU;
		case ast::Binary::Operation::AndI: return Binary::Op::AndI;
		case ast::Binary::Operation::OrI:  return Binary::Op::OrI;
		case ast::Binary::Operation::XorI: return Binary::Op::XorI;
		case ast::Binary::Operation::AddF: return Binary::Op::AddF;
		case ast::Binary::Operation::MulF: return Binary::Op::MulF;
		case ast::Binary::Operation::SubF: return Binary::Op::SubF;
		case ast::Binary::Operation::DivF: return Binary::Op::DivF;
	}
}

static inline std::shared_ptr<Temporary> visitExpression(Context& bbb, std::shared_ptr<const ast::RValue> val)
{
	std::shared_ptr<Temporary> ret;

	val->accept(overloaded
	{
		[&](const ast::Local& v) { ret = bbb.getLocal(v.shared_from_this()); },
		[&](const ast::Global& v) { ret = bbb.addOp([&](auto t){ return std::make_shared<LoadGlobal>(t, v.field); }); },
		[&](const ast::Argument& v) { ret = bbb.arg(v.idx); },
		[&](const ast::Create& v) { ret = bbb.addOp([&](auto t){ return std::make_shared<Create>(t, v.type); }); },
		[&](const ast::Literal& v) { ret = bbb.addOp([&](auto t){ return std::make_shared<Literal>(t, v.integer); }); },
		[&](const ast::Dereference& v)
		{
			const auto in = visitExpression(bbb, v.object);
			ret = bbb.addOp([&](auto t){ return std::make_shared<LoadField>(t, in, v.field); });
		},
		[&](const ast::Unary& v)
		{
			switch(v.op)
			{
				case ast::Unary::Operation::Not:
					// TODO eliminate with jumpy-jumpy
					break;
				default:
				{
					const auto in = visitExpression(bbb, v.arg);
					ret = bbb.addOp([&](auto t){ return std::make_shared<Unary>(t, in, mapUnaryOp(v.op)); });
				}
			}
		},
		[&](const ast::Binary& v)
		{
			switch(v.op)
			{
				case ast::Binary::Operation::Eq:
				case ast::Binary::Operation::Ne:
				case ast::Binary::Operation::LtI:
				case ast::Binary::Operation::GtI:
				case ast::Binary::Operation::LeI:
				case ast::Binary::Operation::GeI:
				case ast::Binary::Operation::LtU:
				case ast::Binary::Operation::GtU:
				case ast::Binary::Operation::LeU:
				case ast::Binary::Operation::GeU:
				case ast::Binary::Operation::LtF:
				case ast::Binary::Operation::GtF:
				case ast::Binary::Operation::LeF:
				case ast::Binary::Operation::GeF:
					// TODO eliminate with jumpy-jumpy
					break;
				case ast::Binary::Operation::And:
					// TODO eliminate with jumpy-jumpy
					break;
				case ast::Binary::Operation::Or:
					// TODO eliminate with jumpy-jumpy
					break;
				default:
				{
					const auto firstIn = visitExpression(bbb, v.first);
					const auto secondIn = visitExpression(bbb, v.second);
					ret = bbb.addOp([&](auto t){ return std::make_shared<Binary>(t, firstIn, secondIn, mapBinaryOp(v.op)); });
				}
			}
		},
		[&](const ast::Set& v)
		{
			ret = visitExpression(bbb, v.value);

			auto target = v.target;

			target->accept(overloaded{
				[&](const ast::Local& d){ target = std::make_shared<Dereference>(visitExpression(bbb, d.object), d.field); },
				[&](const ast::Global& d){ target = std::make_shared<Dereference>(visitExpression(bbb, d.object), d.field); },
				[&](const ast::Argument& d){ target = std::make_shared<Dereference>(visitExpression(bbb, d.object), d.field); },
				[&](const ast::Dereference& d){ target = std::make_shared<Dereference>(visitExpression(bbb, d.object), d.field); },
				[&](const auto& o){}
			});

			bbb.add(std::make_shared<ExpressionStatement>(std::make_shared<Set>(target, ret)));
		},
		[&](const ast::Call& v)
		{
			std::vector<std::shared_ptr<Temporary>> args;
			std::transform(v.args.begin(), v.args.end(), std::back_inserter(args), [&](const auto& v){ return visitExpression(bbb, v); });
			auto call = std::make_shared<Call>(v.fn, std::move(args));
			if(call->fn->ret.size())
			{
				ret = bbb.addTemporary(call);
			}
			else
			{
				bbb.addOp(std::make_shared<ExpressionStatement>(call));
			}
		},
		[&](const ast::Ternary& v) {
			auto t = std::make_shared<Temporary>(v.getType());
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

static inline void visitStatement(Context& bbb, std::shared_ptr<ast::Statement> stmt)
{
	stmt->accept(overloaded
	{
		[&](const ast::ExpressionStatement& v) {
			visitExpression(bbb, v.val);
		},
		[&](const ast::Declaration& v) {
			bbb.addLocal(v.local, visitExpression(bbb, v.initializer));
		},
		[&](const ast::Block& v)
		{
			for(const std::shared_ptr<ast::Statement>& s: v.stmts)
			{
				visitStatement(bbb, s);
			}
		},
		[&](const ast::Return& v)
		{
			std::vector<std::shared_ptr<Temporary>> retvals;
			std::transform(v.value.begin(), v.value.end(), std::back_inserter(retvals), [&](const auto& v){ return visitExpression(bbb, v);});

			// TODO jumpy-jumpy
		},
		[&](const ast::Conditional& v){}, // TODO eliminate with jumpy-jumpy
		[&](const ast::Continue& v){}, // TODO eliminate with jumpy-jumpy
		[&](const ast::Break& v){}, // TODO eliminate with jumpy-jumpy
		[&](const ast::Loop& v){}, // TODO eliminate with jumpy-jumpy
	});
}

std::shared_ptr<ir::Function> ir::Function::from(std::shared_ptr<ast::Function> f)
{
	Context bbb;
	visitStatement(bbb, f->body);
	return std::make_shared<ir::Function>(bbb.bbs);
}

std::string ir::Function::dump(ast::ProgramObjectSet& gi) const
{
	std::stringstream ss;

	auto getIdx = [this](auto i) { return std::find(blocks.begin(), blocks.end(), i) - blocks.begin(); };

	for(auto i = 0u; i < blocks.size(); i++)
	{
		const auto& bb = blocks[i];

		ss << i << ":" << std::endl;
		ss << bb->dump(gi);

		if(bb->decisionInput)
		{
			ss << "if ???" /*<< dumpExpressionAst(gi, locals, bb->decisionInput, OpPrecedence::Root)*/ << " then goto " << getIdx(bb->then) << " else goto " << getIdx(bb->otherwise) << std::endl;
		}
		else if(bb->then)
		{
			ss << "goto " << getIdx(bb->then) << std::endl;
		}
		else
		{
			ss << "done" << std::endl;
		}
	}

	const auto ret = ss.str();
	return ret;
}
