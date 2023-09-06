#include "Compiler.h"

#include "compiler/ast/Values.h"
#include "compiler/ast/Statements.h"

#include "compiler/ir/Function.h"
#include "compiler/ir/IrBuilder.h"

using namespace comp;
using namespace comp::ir;

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
#define X(n, ...) case ast::Binary::Operation:: n: return Binary::Op:: n;
		_ARITHMETIC_OPERATORS(X)
#undef X
	}
}

inline auto mapConditionalOp(ast::Binary::Operation op)
{
	switch(op)
	{
		default:
#define X(n, ...) case ast::Binary::Operation:: n: return Conditional::Condition:: n;
		_CONDITIONAL_OPERATORS(X)
#undef X
	}
}

struct Context: IrBuilder
{
	std::map<std::shared_ptr<const ast::Local>, std::shared_ptr<Variable>> locals;
	std::vector<std::shared_ptr<Variable>> args;

	Context(std::vector<ast::ValueType> argTypes)
	{
		std::transform(argTypes.begin(), argTypes.end(), std::back_inserter(args), [&](auto& t){ return std::make_shared<Variable>(t); });
	}

	void addLocal(std::shared_ptr<const ast::Local> local, std::shared_ptr<Variable> t)
	{
		const bool inserted = locals.insert({local, t}).second;
		assert(inserted);
	}

	std::shared_ptr<Variable> getLocal(std::shared_ptr<const ast::Local> local)
	{
		const auto it = locals.find(local);
		assert(it != locals.end());
		return it->second;
	}

	std::shared_ptr<Variable> arg(size_t idx)
	{
		assert(idx < args.size());
		return args[idx];
	}

	inline std::shared_ptr<Variable> operator()(std::shared_ptr<const ast::RValue> val, std::shared_ptr<Variable> ret = {})
	{
		if(!ret)
		{
			ret = std::make_shared<Variable>(val->getType());
		}

		val->accept(overloaded
		{
			[&](const ast::Local& v) { ret = getLocal(v.shared_from_this()); },
			[&](const ast::Global& v) { addOp(std::make_shared<LoadGlobal>(ret, v.field)); },
			[&](const ast::Argument& v) { ret = arg(v.idx); },
			[&](const ast::Create& v) { addOp(std::make_shared<Create>(ret, v.type)); },
			[&](const ast::Literal& v) { addOp(std::make_shared<Copy>(ret, std::make_shared<Constant>(v.getType(), v.integer))); },
			[&](const ast::Dereference& v) { addOp(std::make_shared<LoadField>(ret, (*this)(v.object), v.field)); },
			[&](const ast::Unary& v)
			{
				if(v.op == ast::Unary::Operation::Not)
				{
					branch((*this)(v.arg), [&](){ addLiteral(ret, 0); }, [&](){ addLiteral(ret, 1); });
				}
				else
				{
					addOp(std::make_shared<Unary>(ret, (*this)(v.arg), mapUnaryOp(v.op)));
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
						condToBool(ret, mapConditionalOp(v.op), (*this)(v.first), (*this)(v.second));
						break;
					case ast::Binary::Operation::And:
						branch((*this)(v.first), [&](){ addOp(std::make_shared<Copy>(ret, (*this)(v.second))); }, [&](){ addLiteral(ret, 0); });
						break;
					case ast::Binary::Operation::Or:
						branch((*this)(v.first), [&](){ addLiteral(ret, 1); }, [&](){ addOp(std::make_shared<Copy>(ret, (*this)(v.second))); });
						break;
					default:
						addOp(std::make_shared<Binary>(ret, (*this)(v.first), (*this)(v.second), mapBinaryOp(v.op)));
				}
			},
			[&](const ast::Ternary& v) {
				branch((*this)(v.condition), [&](){ addOp(std::make_shared<Copy>(ret, (*this)(v.then))); }, [&](){ addOp(std::make_shared<Copy>(ret, (*this)(v.otherwise))); });
			},
			[&](const ast::Call& v)
			{
				std::vector<std::shared_ptr<Temporary>> args;
				std::transform(v.args.begin(), v.args.end(), std::back_inserter(args), [&](const auto& v){ return (*this)(v); });

				std::vector<std::shared_ptr<Variable>> out;
				if(!v.fn->ret.empty())
				{
					assert(v.fn->ret.size() == 1);
					out.push_back(ret);
				}

				addOp(std::make_shared<Call>(args, out, v.fn));
			},
			[&](const ast::Set& v)
			{
				v.target->accept(overloaded
				{
					[&](const ast::Local& d) { ret = (*this)(v.value, getLocal(d.shared_from_this())); },
					[&](const ast::Argument& d){ ret = (*this)(v.value, arg(d.idx)); },
					[&](const ast::Global& d) {
						ret = (*this)(v.value);
						addOp(std::make_shared<StoreGlobal>(ret, d.field));
					},
					[&](const ast::Dereference& d)
					{
						const auto object = (*this)(d.object);
						ret = (*this)(v.value);
						addOp(std::make_shared<StoreField>(ret, object, d.field));
					},
					[&](const ast::RValue& o){ assert(false); }
				});
			},
		});

		return ret;
	}

	inline void operator()(std::shared_ptr<ast::Statement> stmt)
	{
		stmt->accept(overloaded
		{
			[&](const ast::ExpressionStatement& v) {
				(*this)(v.val);
			},
			[&](const ast::Declaration& v) {
				addLocal(v.local, (*this)(v.initializer));
			},
			[&](const ast::Block& v)
			{
				for(const std::shared_ptr<ast::Statement>& s: v.stmts)
				{
					(*this)(s);
				}
			},
			[&](const ast::Return& v)
			{
				std::vector<std::shared_ptr<Temporary>> retvals;
				std::transform(v.value.begin(), v.value.end(), std::back_inserter(retvals), [&](const auto& v){ return (*this)(v);});
				leave(retvals);
			},
			[&](const ast::Conditional& v)
			{
				branch((*this)(v.condition),
				[&](){ (*this)(v.then); },
				[&](){ (*this)(v.otherwise); });
			},
			[&](const ast::Loop& v)
			{
				loop(&v, [&]()
				{
					(*this)(v.body);
				});
			},
			[&](const ast::Continue& v) { continueLoop(v.loop.get()); },
			[&](const ast::Break& v) { breakLoop(v.loop.get()); },
		});
	}
};

std::shared_ptr<ir::Function> Compiler::generateIr(std::shared_ptr<ast::Function> f)
{
	Context ctx(f->args);
	ctx(f->body);
	return std::make_shared<ir::Function>(ctx.args, ctx.build());
}
