#include "Compiler.h"

#include "compiler/ir/Temporary.h"
#include "compiler/ir/Operations.h"
#include "compiler/ir/Terminations.h"

#include "compiler/common/Annotations.h"

#include "Overloaded.h"

#include <set>
#include <sstream>

using namespace comp;
using namespace comp::ir;

struct LivenessDelta
{
	std::vector<std::shared_ptr<Variable>> read, written;

	inline void addRead(std::shared_ptr<Temporary> t)
	{
		if(auto v = std::dynamic_pointer_cast<Variable>(t))
		{
			read.push_back(v);
		}
	}

	inline void addWrite(std::shared_ptr<Variable> v) {
		written.push_back(v);
	}
};

struct LivenessAnalysis
{
	std::set<std::shared_ptr<Temporary>> liveVariables;

	std::string asCommentText(BasicBlock::DumpContext& dc) const
	{
		const char* sep = "";
		std::stringstream ss;

		for(const auto& p: liveVariables)
		{
			ss << sep << dc.nameOf(p);
			sep = ", ";
		}

		const auto ret = ss.str();
		return ret;
	}

	bool isLive(std::shared_ptr<Variable> v) {
		return liveVariables.find(v) != liveVariables.end();
	}

	void apply(const LivenessDelta& delta)
	{
		std::for_each(delta.read.begin(), delta.read.end(), [&](const auto &v){ liveVariables.insert(v); });
		std::for_each(delta.written.begin(), delta.written.end(), [&](const auto &v){ liveVariables.erase(v); });
	}
};

static inline LivenessDelta getDelta(const std::shared_ptr<ir::Operation> &op)
{
	LivenessDelta ret;

	op->accept(overloaded
	{
		[&](const Create& v)
		{
			ret.addWrite(v.target);
		},
		[&](const LoadGlobal& v)
		{
			ret.addWrite(v.target);
		},
		[&](const StoreGlobal& v)
		{
			ret.addRead(v.source);
		},
		[&](const StoreField& v)
		{
			ret.addRead(v.object);
			ret.addRead(v.source);
		},
		[&](const LoadField& v)
		{
			ret.addRead(v.object);
			ret.addWrite(v.target);
		},
		[&](const Copy& v)
		{
			ret.addRead(v.source);
			ret.addWrite(v.target);
		},
		[&](const Unary& v)
		{
			ret.addRead(v.source);
			ret.addWrite(v.target);
		},
		[&](const Binary& v)
		{
			ret.addRead(v.first);
			ret.addRead(v.second);
			ret.addWrite(v.target);
		},
		[&](const Call& v)
		{
			std::for_each(v.arg.begin(), v.arg.end(), [&](const auto &a){ret.addRead(a);});
			std::for_each(v.ret.begin(), v.ret.end(), [&](const auto &r){ret.addWrite(r);});
		},
	});

	return ret;
}

static inline LivenessAnalysis calculateAtExitPoint(const std::map<std::shared_ptr<BasicBlock>, LivenessAnalysis> &anal, const std::shared_ptr<ir::Termination> &t)
{
	LivenessAnalysis ret;

	t->accept(overloaded
	{
		[&](const Leave& v)
		{
			LivenessDelta d;
			std::for_each(v.ret.begin(), v.ret.end(), [&](const auto& r){d.addRead(r);});
			ret.apply(d);
		},
		[&](const Always& v)
		{
			if(auto it = anal.find(v.continuation); it != anal.end())
			{
				ret = it->second;
			}
		},
		[&](const Conditional& v)
		{
			if(auto it = anal.find(v.then); it != anal.end())
			{
				ret = it->second;
			}

			if(auto it = anal.find(v.otherwise); it != anal.end())
			{
				LivenessDelta d;
				std::for_each(it->second.liveVariables.begin(), it->second.liveVariables.end(), [&](const auto& r){d.addRead(r);});
				ret.apply(d);
			}

			LivenessDelta d;
			d.addRead(v.first);
			d.addRead(v.second);
			ret.apply(d);
		}
	});

	return ret;
}

static inline bool removeUselessOpeations(const std::map<std::shared_ptr<BasicBlock>, LivenessAnalysis> &anal, const std::shared_ptr<ir::Function> &f)
{
	bool ret = false;

	f->traverse([&](std::shared_ptr<BasicBlock> bb)
	{
		LivenessAnalysis state = calculateAtExitPoint(anal, bb->termination);

		for(auto it = bb->code.rbegin(); it != bb->code.rend(); it++)
		{
			const std::shared_ptr<Operation>& o = *it;
			const auto d = getDelta(o);

			if(std::dynamic_pointer_cast<Create>(o) == nullptr &&
				std::dynamic_pointer_cast<StoreField>(o) == nullptr &&
				std::dynamic_pointer_cast<StoreGlobal>(o) == nullptr &&
				std::dynamic_pointer_cast<Call>(o) == nullptr)
			{
				if(std::none_of(d.written.begin(), d.written.end(), [&](const auto &v){ return state.isLive(v); }))
				{
					bb->code.erase(std::next(it).base());
					ret = true;
				}
			}

			state.apply(d);
		}
	});

	return ret;
}

static inline std::map<std::shared_ptr<BasicBlock>, LivenessAnalysis> runAnalysis(const std::shared_ptr<ir::Function> &f)
{
	bool changed;
	std::map<std::shared_ptr<BasicBlock>, LivenessAnalysis> ret;

	do
	{
		changed = false;

		f->traverse([&](std::shared_ptr<BasicBlock> bb)
		{
			LivenessAnalysis state = calculateAtExitPoint(ret, bb->termination);

			std::for_each(bb->code.rbegin(), bb->code.rend(), [&](const auto& o){ state.apply(getDelta(o)); });

			if(ret[bb].liveVariables != state.liveVariables)
			{
				ret[bb] = state;
				changed = true;
			}
		});

	} while(changed);

	return ret;
}

bool Compiler::eliminateDeadCode(std::shared_ptr<ir::Function> f)
{
	bool ret = false;

	while(removeUselessOpeations(runAnalysis(f), f))
	{
		ret = true;
	}

	return ret;
}
