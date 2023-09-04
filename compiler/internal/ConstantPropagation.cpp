#include "Compiler.h"

#include "compiler/ir/Temporary.h"
#include "compiler/ir/Operations.h"
#include "compiler/ir/Terminations.h"

#include "compiler/common/Annotations.h"

#include "Overloaded.h"

#include <map>
#include <sstream>

using namespace comp;
using namespace comp::ir;

static inline float i2f(int i) { return *(float*)&i; }
static inline int f2i(float f) { return *(int*)&f; }

static inline bool evaluateCondition(Conditional::Condition cond, int a, int b)
{
	switch(cond)
	{
		case Conditional::Condition::Eq: return a == b;
		case Conditional::Condition::Ne: return a != b;
		case Conditional::Condition::LtI: return a < b;
		case Conditional::Condition::GtI: return a > b;
		case Conditional::Condition::LeI: return a <= b;
		case Conditional::Condition::GeI: return a >= b;
		case Conditional::Condition::LtU: return (unsigned)a < (unsigned)b;
		case Conditional::Condition::GtU: return (unsigned)a > (unsigned)b;
		case Conditional::Condition::LeU: return (unsigned)a <= (unsigned)b;
		case Conditional::Condition::GeU: return (unsigned)a >= (unsigned)b;
		case Conditional::Condition::LtF: return i2f(a) < i2f(b);
		case Conditional::Condition::GtF: return i2f(a) > i2f(b);
		case Conditional::Condition::LeF: return i2f(a) <= i2f(b);
		case Conditional::Condition::GeF: return i2f(a) >= i2f(b);
	}

	assert(false);
	return false;
}

int evaluateBinary(Binary::Op op, int f, int s)
{
	switch(op)
	{
		case Binary::Op::AddI: return f + s;
		case Binary::Op::MulI: return f * s;
		case Binary::Op::SubI: return f - s;
		case Binary::Op::DivI: return f / s;
		case Binary::Op::Mod : return f % s;
		case Binary::Op::ShlI: return f << s;
		case Binary::Op::ShrI: return f >> s;
		case Binary::Op::ShrU: return (unsigned)f >> s;
		case Binary::Op::AndI: return f & s;
		case Binary::Op::OrI : return f | s;
		case Binary::Op::XorI: return f ^ s;
		case Binary::Op::AddF: return f2i(i2f(f) + i2f(s));
		case Binary::Op::MulF: return f2i(i2f(f) * i2f(s));
		case Binary::Op::SubF: return f2i(i2f(f) - i2f(s));
		case Binary::Op::DivF: return f2i(i2f(f) / i2f(s));
	}

	assert(false);
	return 0;
}

class BlockAnalysis
{
	std::map<std::shared_ptr<Temporary>, std::optional<int>> data;

public:
	std::optional<int> get(std::shared_ptr<Temporary> t) const
	{
		if(auto it = data.find(t); it != data.end())
		{
			return it->second;
		}

		return {};
	}

	void add(std::shared_ptr<Temporary> t, std::optional<int> v)
	{
		assert(!t->isConstant());
		data[t] = v;
	}

	std::optional<int> eval(std::shared_ptr<Temporary> t) const
	{
		if(auto c = std::dynamic_pointer_cast<Constant>(t))
		{
			return c->value;
		}

		return get(t);
	}

	void eval(std::shared_ptr<Operation> op)
	{
		op->accept(overloaded
		{
			[&](const StoreField& v) {},
			[&](const StoreGlobal& v) {},
			[&](const Create& v)
			{
				add(v.target, {});
			},
			[&](const LoadField& v)
			{
				add(v.target, {});
			},
			[&](const LoadGlobal& v)
			{
				add(v.target, {});
			},
			[&](const Call& v)
			{
				std::for_each(v.ret.begin(), v.ret.end(), [&](const auto r){ add(r, {}); });
			},
			[&](const Copy& v)
			{
				if(auto val = eval(v.source))
				{
					add(v.target, *val);
				}
				else
				{
					add(v.target, {});
				}
			},
			[&](const Unary& v)
			{
				if(auto c = eval(v.source))
				{
					switch(v.op)
					{
						case Unary::Op::Neg:
							add(v.target, ~*c);
						break;
						case Unary::Op::I2F:
						case Unary::Op::F2I:
							add(v.target, *c);
						break;
						default: assert(false);
					}

				}
				else
				{
					add(v.target, {});
				}
			},
			[&](const Binary& v)
			{
				auto f = eval(v.first);
				auto s = eval(v.second);

				if(f.has_value() && s.has_value())
				{
					add(v.target, evaluateBinary(v.op, *f, *s));
				}
				else
				{
					add(v.target, {});
				}
			},
		});
	}

	std::string asCommentText(BasicBlock::DumpContext& dc) const
	{
		const char* sep = "";
		std::stringstream ss;

		for(const auto& p: data)
		{
			ss << sep << dc.nameOf(p.first) << ":" << (p.second ? std::to_string(*p.second) : "-");
			sep = " ";
		}

		const auto ret = ss.str();
		return ret;
	}

	bool mergeWith(const BlockAnalysis& other)
	{
		bool ret = false;

		for(const auto &p: other.data)
		{
			if(auto it = data.find(p.first); it != data.end())
			{
				if(p.second != it->second)
				{
					if(it->second != std::nullopt)
					{
						it->second = std::nullopt;
						ret = true;
					}
				}
			}
			else
			{
				data.insert(p);
				ret = true;
			}
		}

		return ret;
	}

	std::shared_ptr<Operation> substitute(std::shared_ptr<Operation> o) const
	{
		std::shared_ptr<Operation> ret = o;

		o->accept(overloaded
		{
			[&](const StoreField& v) {},
			[&](const StoreGlobal& v) {},
			[&](const Create& v) {},
			[&](const LoadField& v) {},
			[&](const LoadGlobal& v) {},
			[&](const Unary& v) { /* should be processed as constant already if its only arg is constant */ },
			[&](const Call& v)
			{
				bool changed = false;
				std::vector<std::shared_ptr<Temporary>> mapped;

				std::for_each(v.arg.begin(), v.arg.end(), [&](const auto &a)
				{
					if(auto v = get(a))
					{
						changed = true;
						mapped.push_back(std::make_shared<Constant>(a->type, *v));
					}
					else
					{
						mapped.push_back(a);
					}
				});

				if(changed)
				{
					ret = std::make_shared<Call>(mapped, v.ret, v.fn);
				}
			},
			[&](const Copy& v)
			{
				if(auto val = get(v.source))
				{
					ret = std::make_shared<Copy>(v.target, std::make_shared<Constant>(v.source->type, *val));
				}
			},
			[&](const Binary& v)
			{
				if(auto f = get(v.first))
				{
					/* should be processed as constant already if both args are constant */
					ret = std::make_shared<Binary>(v.target, std::make_shared<Constant>(v.first->type, *f), v.second, v.op);
				}

				if(auto s = get(v.second))
				{
					ret = std::make_shared<Binary>(v.target, v.first, std::make_shared<Constant>(v.first->type, *s), v.op);
				}
			},
		});

		return ret;
	}

	std::shared_ptr<Termination> substitute(std::shared_ptr<Termination> t) const
	{
		std::shared_ptr<Termination> ret = t;

		t->accept(overloaded
		{
			[&](const Leave& v) {},
			[&](const Always& v) {},
			[&](const Conditional& v)
			{
				auto f = eval(v.first);
				auto s = eval(v.second);

				if(f.has_value() && s.has_value())
				{
					ret = std::make_shared<Always>(evaluateCondition(v.condition, *f, *s) ? v.then : v.otherwise);
				}
				else if(f.has_value() && !v.first->isConstant())
				{
					ret = std::make_shared<Conditional>(v.condition, std::make_shared<Constant>(v.first->type, *f), v.second, v.then, v.otherwise);
				}
				else if(s.has_value() && !v.second->isConstant())
				{
					ret = std::make_shared<Conditional>(v.condition, v.first, std::make_shared<Constant>(v.second->type, *s), v.then, v.otherwise);
				}
			}
		});

		return ret;
	}
};

static inline std::map<std::shared_ptr<BasicBlock>, BlockAnalysis> runAnalysis(std::shared_ptr<ir::Function> f)
{
	bool changed;
	std::map<std::shared_ptr<BasicBlock>, BlockAnalysis> ret;

	do
	{
		changed = false;

		f->traverse([&](std::shared_ptr<BasicBlock> bb)
		{
			BlockAnalysis state = ret[bb];
			std::for_each(bb->code.begin(), bb->code.end(), [&](const auto& o){ state.eval(o); });

			bb->termination->accept(overloaded
			{
				[&](const Leave& v){},
				[&](const Always& v)
				{
					changed = changed || ret[v.continuation].mergeWith(state);
				},
				[&](const Conditional& v)
				{
					auto f = state.eval(v.first);
					auto s = state.eval(v.second);

					if(f.has_value() && s.has_value())
					{
						if(evaluateCondition(v.condition, *f, *s))
						{
							changed = changed || ret[v.then].mergeWith(state);
						}
						else
						{
							changed = changed || ret[v.otherwise].mergeWith(state);
						}
					}
					else
					{
						changed = changed || ret[v.then].mergeWith(state);
						changed = changed || ret[v.otherwise].mergeWith(state);
					}
				}
			});
		});
	} while(changed);

	return ret;
}

static inline bool substitute(const std::map<std::shared_ptr<BasicBlock>, BlockAnalysis> &anal, std::shared_ptr<ir::Function> f)
{
	bool ret = false;
	BasicBlock::DumpContext dc;
	for(const auto &a: f->args) dc.nameOf(a);

	f->traverse([&](std::shared_ptr<BasicBlock> bb)
	{
		BlockAnalysis state = anal.find(bb)->second;

		std::for_each(bb->code.begin(), bb->code.end(), [&](auto& o)
		{
			state.eval(o);

			if(auto s = state.substitute(o); o != s)
			{
				o = s;
				ret = true;
			}
		});

//		bb->annotations.push_back(std::make_shared<IrComment>([state](auto& dc){ return state.asCommentText(dc);}));

		if(auto t = state.substitute(bb->termination); bb->termination != t)
		{
			bb->termination = t;
			ret = true;
		}
	});

	return ret;
}

bool Compiler::propagateConstants(std::shared_ptr<ir::Function> f)
{
	bool ret = false;

	while(substitute(runAnalysis(f), f))
	{
		ret = true;
	}

	return ret;
}
