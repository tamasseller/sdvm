#include "Function.h"

#include "Operations.h"

#include "compiler/ast/Values.h"
#include "compiler/ast/Statements.h"

#include "Terminations.h"
#include "Overloaded.h"
#include "assert.h"

#include <map>
#include <set>
#include <sstream>
#include <algorithm>

using namespace comp;
using namespace comp::ir;

struct BuilderContext
{
	struct LoopInfo
	{
		std::shared_ptr<BasicBlock> start;
		std::vector<std::function<void(std::shared_ptr<BasicBlock>)>> endConsumers;

		inline LoopInfo(decltype(start) start): start(start) {}
	};

	std::vector<std::shared_ptr<BasicBlock>> bbs;
	std::map<std::shared_ptr<const ast::Local>, std::shared_ptr<Temporary>> locals;
	std::vector<std::shared_ptr<Temporary>> args;
	std::map<const void*, LoopInfo> loops;

	BuilderContext(std::vector<ast::ValueType> argTypes)
	{
		std::transform(argTypes.begin(), argTypes.end(), std::back_inserter(args), [&](auto& t){ return std::make_shared<Temporary>(t); });
		bbs.push_back(std::make_shared<BasicBlock>());
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

	void addOp(std::shared_ptr<Operation> stmt) {
		bbs.back()->code.push_back(stmt);
	}

	template<class C>
	auto genOp(ast::ValueType t, C&& c)
	{
		auto ret = std::make_shared<Temporary>(t);
		bbs.back()->code.push_back(c(ret));
		return ret;
	}

	std::pair<std::shared_ptr<BasicBlock>, std::shared_ptr<BasicBlock>> close()
	{
		auto old = bbs.back();
		bbs.push_back(std::make_shared<BasicBlock>());
		return {old, bbs.back()};
	}

	std::shared_ptr<Temporary> condToBool(Conditional::Condition condition, std::shared_ptr<Temporary> first, std::shared_ptr<Temporary> second)
	{
		auto ret = std::make_shared<Temporary>(ast::ValueType::logical());

		auto ifThenPoint = close();
		addOp(std::make_shared<Literal>(ret, 1));
		auto thenElsePoint = close();
		addOp(std::make_shared<Literal>(ret, 0));
		auto endifPoint = close();

		ifThenPoint.first->termination = std::make_shared<Conditional>(condition, first, second, ifThenPoint.second, thenElsePoint.second);
		thenElsePoint.first->termination = std::make_shared<Always>(endifPoint.second);
		endifPoint.first->termination = std::make_shared<Always>(endifPoint.second);

		return ret;
	}

	template<class T, class O>
	void branch(std::shared_ptr<Temporary> decisionInput, T&& then, O&& otherwise)
	{
		auto tru = genOp(ast::ValueType::logical(), [&](auto t){ return std::make_shared<Literal>(t, 1); });

		auto ifThenPoint = close();
		then();
		auto thenElsePoint = close();
		otherwise();
		auto endifPoint = close();

		ifThenPoint.first->termination = std::make_shared<Conditional>(Conditional::Condition::Eq, decisionInput, tru, ifThenPoint.second, thenElsePoint.second);
		thenElsePoint.first->termination = std::make_shared<Always>(endifPoint.second);
		endifPoint.first->termination = std::make_shared<Always>(endifPoint.second);
	}

	std::shared_ptr<ir::Function> build()
	{
		assert(bbs.back()->code.empty());
		assert(!bbs.back()->termination);
		bbs.pop_back();

		std::set<std::shared_ptr<BasicBlock>> toDo{bbs.front()}, reachable;

		while(!toDo.empty())
		{
			const auto current = *toDo.begin();
			toDo.erase(current);

			if(reachable.insert(current).second)
			{
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

		for(auto it = bbs.begin(); it != bbs.end();)
		{
			if(reachable.find(*it) == reachable.end())
			{
				it = bbs.erase(it);
			}
			else
			{
				it++;
			}
		}

		return std::make_shared<ir::Function>(bbs);
	}

	template<class C>
	void loop(const void* loopIdentity, C&& c)
	{
		auto startPoint = close();
		startPoint.first->termination = std::make_shared<Always>(startPoint.second);

		auto l = loops.insert({loopIdentity, LoopInfo(startPoint.second)});
		assert(l.second);

		c();

		auto endPoint = close();
		endPoint.first->termination = std::make_shared<Always>(startPoint.second);

		for(const auto& cb: l.first->second.endConsumers)
		{
			cb(endPoint.second);
		}

		loops.erase(l.first);
	}

	void continueLoop(const void* loopIdentity)
	{
		auto it = loops.find(loopIdentity);
		assert(it != loops.end()); // TODO compiler error: continue outside loop

		auto cutPoint = close();
		cutPoint.first->termination = std::make_shared<Always>(it->second.start);
	}

	void breakLoop(const void* loopIdentity)
	{
		auto it = loops.find(loopIdentity);
		assert(it != loops.end()); // TODO compiler error: continue outside loop

		auto cutPoint = close();
		it->second.endConsumers.push_back([first{cutPoint.first}](auto end){
			first->termination = std::make_shared<Always>(end);
		});
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

inline auto mapConditionalOp(ast::Binary::Operation op)
{
	switch(op)
	{
		default:
		case ast::Binary::Operation::Eq: return Conditional::Condition::Eq;
		case ast::Binary::Operation::Ne: return Conditional::Condition::Ne;
		case ast::Binary::Operation::LtI: return Conditional::Condition::LtI;
		case ast::Binary::Operation::GtI: return Conditional::Condition::GtI;
		case ast::Binary::Operation::LeI: return Conditional::Condition::LeI;
		case ast::Binary::Operation::GeI: return Conditional::Condition::GeI;
		case ast::Binary::Operation::LtU: return Conditional::Condition::LtU;
		case ast::Binary::Operation::GtU: return Conditional::Condition::GtU;
		case ast::Binary::Operation::LeU: return Conditional::Condition::LeU;
		case ast::Binary::Operation::GeU: return Conditional::Condition::GeU;
		case ast::Binary::Operation::LtF: return Conditional::Condition::LtF;
		case ast::Binary::Operation::GtF: return Conditional::Condition::GtF;
		case ast::Binary::Operation::LeF: return Conditional::Condition::LeF;
		case ast::Binary::Operation::GeF: return Conditional::Condition::GeF;
	}
}

static inline std::shared_ptr<Temporary> visitExpression(BuilderContext& bbb, std::shared_ptr<const ast::RValue> val)
{
	std::shared_ptr<Temporary> ret;

	val->accept(overloaded
	{
		[&](const ast::Local& v) { ret = bbb.getLocal(v.shared_from_this()); },
		[&](const ast::Global& v) { ret = bbb.genOp(v.getType(), [&](auto t){ return std::make_shared<LoadGlobal>(t, v.field); }); },
		[&](const ast::Argument& v) { ret = bbb.arg(v.idx); },
		[&](const ast::Create& v) { ret = bbb.genOp(v.getType(), [&](auto t){ return std::make_shared<Create>(t, v.type); }); },
		[&](const ast::Literal& v) { ret = bbb.genOp(v.getType(), [&](auto t){ return std::make_shared<Literal>(t, v.integer); }); },
		[&](const ast::Dereference& v)
		{
			const auto in = visitExpression(bbb, v.object);
			ret = bbb.genOp(v.getType(), [&](auto t){ return std::make_shared<LoadField>(t, in, v.field); });
		},
		[&](const ast::Unary& v)
		{
			switch(v.op)
			{
				case ast::Unary::Operation::Not:
					ret = std::make_shared<Temporary>(v.getType());
					bbb.branch(visitExpression(bbb, v.arg),
							[&](){ bbb.addOp(std::make_shared<Literal>(ret, 1)); },
							[&](){ bbb.addOp(std::make_shared<Literal>(ret, 0)); });

					break;
				default:
				{
					const auto in = visitExpression(bbb, v.arg);
					ret = bbb.genOp(v.getType(), [&](auto t){ return std::make_shared<Unary>(t, in, mapUnaryOp(v.op)); });
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
					ret = bbb.condToBool(mapConditionalOp(v.op), visitExpression(bbb, v.first), visitExpression(bbb, v.second));
					break;
				case ast::Binary::Operation::And:
					ret = std::make_shared<Temporary>(ast::ValueType::logical());
					bbb.branch(visitExpression(bbb, v.first),
							[&](){ bbb.addOp(std::make_shared<Copy>(ret, visitExpression(bbb, v.second))); },
							[&](){ bbb.addOp(std::make_shared<Literal>(ret, 0)); });

					break;
				case ast::Binary::Operation::Or:
					ret = std::make_shared<Temporary>(ast::ValueType::logical());
					bbb.branch(visitExpression(bbb, v.first),
							[&](){ bbb.addOp(std::make_shared<Literal>(ret, 1)); },
							[&](){ bbb.addOp(std::make_shared<Copy>(ret, visitExpression(bbb, v.second))); });

					break;
				default:
				{
					const auto firstIn = visitExpression(bbb, v.first);
					const auto secondIn = visitExpression(bbb, v.second);
					ret = bbb.genOp(v.getType(), [&](auto t){ return std::make_shared<Binary>(t, firstIn, secondIn, mapBinaryOp(v.op)); });
				}
			}
		},
		[&](const ast::Set& v)
		{
			ret = visitExpression(bbb, v.value);

			v.target->accept(overloaded{
				[&](const ast::Local& d){ bbb.addOp(std::make_shared<Copy>(bbb.getLocal(d.shared_from_this()), ret)); },
				[&](const ast::Global& d){ bbb.addOp(std::make_shared<StoreGlobal>(ret, d.field)); },
				[&](const ast::Argument& d){ bbb.addOp(std::make_shared<Copy>(bbb.arg(d.idx), ret)); },
				[&](const ast::Dereference& d)
				{
					const auto object = visitExpression(bbb, d.object);
					bbb.addOp(std::make_shared<StoreField>(ret, object, d.field));
				},
				[&](const ast::RValue& o){}
			});
		},
		[&](const ast::Call& v)
		{
			std::vector<std::shared_ptr<Temporary>> args, out;
			std::transform(v.args.begin(), v.args.end(), std::back_inserter(args), [&](const auto& v){ return visitExpression(bbb, v); });
			std::transform(v.fn->ret.begin(), v.fn->ret.end(), std::back_inserter(out), [&](const auto& v){ return std::make_shared<Temporary>(v); });

			bbb.addOp(std::make_shared<Call>(args, out, v.fn));

			if(!v.fn->ret.empty())
			{
				assert(v.fn->ret.size() == 1);
				ret = out[0];
			}
		},
		[&](const ast::Ternary& v)
		{
			ret = std::make_shared<Temporary>(v.getType());
			bbb.branch(visitExpression(bbb, v.condition),
					[&](){ bbb.addOp(std::make_shared<Copy>(ret, visitExpression(bbb, v.then))); },
					[&](){ bbb.addOp(std::make_shared<Copy>(ret, visitExpression(bbb, v.otherwise))); });
		},
	});

	return ret;
}

static inline void visitStatement(BuilderContext& bbb, std::shared_ptr<ast::Statement> stmt)
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
			bbb.close().first->termination = std::make_shared<Leave>(retvals);
		},
		[&](const ast::Conditional& v)
		{
			bbb.branch(visitExpression(bbb, v.condition),
			[&](){ visitStatement(bbb, v.then); },
			[&](){ visitStatement(bbb, v.otherwise); });
		},
		[&](const ast::Loop& v)
		{
			bbb.loop(&v, [&]()
			{
				visitStatement(bbb, v.body);
			});
		},
		[&](const ast::Continue& v) { bbb.continueLoop(v.loop.get()); },
		[&](const ast::Break& v) { bbb.breakLoop(v.loop.get()); },
	});
}

std::shared_ptr<ir::Function> ir::Function::from(std::shared_ptr<ast::Function> f)
{
	BuilderContext bbb(f->args);
	visitStatement(bbb, f->body);
	return bbb.build();
}

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

std::string ir::Function::dump(ast::ProgramObjectSet& gi) const
{
	std::stringstream ss;

	auto getIdx = [this](auto i) { return std::find(blocks.begin(), blocks.end(), i) - blocks.begin(); };

	BasicBlock::DumpContext dc;
	for(auto i = 0u; i < blocks.size(); i++)
	{
		const auto& bb = blocks[i];

		ss << i << ":" << std::endl;
		ss << bb->dump(gi, dc);

		bb->termination->accept(overloaded
		{
			[&](const Always& v) {
				ss << "next goto " << getIdx(v.continuation) << std::endl;
			},
			[&](const Conditional& v)
			{
				ss << "if " << dc.nameOf(v.first) << " " << conditionOp.find(v.condition)->second << " " << dc.nameOf(v.second) <<
						" goto " << getIdx(v.then) << " else goto " << getIdx(v.otherwise) << std::endl;
			},
			[&](const Leave& v)
			{
				ss << "exit";

				if(!v.ret.empty()) {
					assert(v.ret.size() == 1);
					ss << " and return " << dc.nameOf(v.ret[0]);
				}

				ss << std::endl;
			},
		});

	}

	const auto ret = ss.str();
	return ret;
}
