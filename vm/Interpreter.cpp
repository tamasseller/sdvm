#include "Program.h"

#include <stdexcept>
#include <memory>

struct State
{
	uint32_t *fp, *sp;
};

static inline uint32_t pop(const Program::FrameInfo& info, State& state)
{
	if(state.sp <= state.fp + info.nLocals)
	{
		throw std::runtime_error("OpStack underflow");
	}

	return *--state.sp;
}

static inline void push(const Program::FrameInfo& info, State& state, uint32_t v)
{
	if(state.fp + info.nLocals + info.maxStack <= state.sp)
	{
		throw std::runtime_error("OpStack overflow");
	}

	*state.sp++ = v;
}

static inline uint32_t loadLocal(const Program::FrameInfo& info, State& state, uint32_t idx)
{
	if(info.nLocals <= idx)
	{
		throw std::runtime_error("Local load out of bounds (" + std::to_string(idx) + " should be below " + std::to_string(info.nLocals) + ")");
	}

	return state.fp[idx];
}

inline void storeLocal(const Program::FrameInfo& info, State& state, uint32_t idx, uint32_t v)
{
	if(info.nLocals <= idx)
	{
		throw std::runtime_error("Local store out of bounds (" + std::to_string(idx) + " should be below " + std::to_string(info.nLocals) + ")");
	}

	state.fp[idx] = v;
}

inline uint32_t loadArg(const Program::FrameInfo& info, State& state, uint32_t idx)
{
	if(info.nArgs <= idx)
	{
		throw std::runtime_error("Argument load out of bounds (" + std::to_string(idx) + " should be below " + std::to_string(info.nArgs) + ")");
	}

	return state.fp[(int32_t)(idx - info.nArgs)];
}

inline void storeArg(const Program::FrameInfo& info, State& state, uint32_t idx, uint32_t v)
{
	if(info.nArgs <= idx)
	{
		throw std::runtime_error("Argument store out of bounds (" + std::to_string(idx) + " should be below " + std::to_string(info.nArgs) + ")");
	}

	state.fp[(int32_t)(idx - info.nArgs)] = v;
}

struct Reader
{
	const Program& p;
	decltype(p.functions.cbegin()) fIt;
	decltype(fIt->body.cbegin()) bIt;
	decltype(bIt->cbegin()) iIt;

	Reader(const Program& p, Program::FrameInfo &fInfo): p(p)
	{
		fIt = p.functions.cbegin();
		bIt = fIt->body.cbegin();
		iIt = bIt->cbegin();

		fInfo = fIt->info;
	}

	struct RestorePoint
	{
		uint32_t fIdx, bIdx, iIdx;
	};

	RestorePoint openFunction(uint32_t idx, Program::FrameInfo &fInfo)
	{
		if(p.functions.size() <= idx)
		{
			throw std::runtime_error("Unknown function called: " + std::to_string(idx) + " (should be below " + std::to_string(p.functions.size()));
		}

		RestorePoint ret
		{
			(uint32_t)(fIt - p.functions.cbegin()),
			(uint32_t)(bIt - fIt->body.cbegin()),
			(uint32_t)(iIt - bIt->cbegin())
		};

		fIt = p.functions.cbegin() + idx;
		bIt = fIt->body.cbegin();
		iIt = bIt->cbegin();

		fInfo = fIt->info;
		return ret;
	}

	void restore(RestorePoint& rp)
	{
		fIt = p.functions.cbegin() + rp.fIdx;
		bIt = fIt->body.cbegin() + rp.bIdx;
		iIt = bIt->cbegin() + rp.iIdx;
	}

	void seekBlock(uint32_t idx)
	{
		if(fIt->body.size() <= idx)
		{
			throw std::runtime_error("Jump to unknown block: " + std::to_string(idx) + " (should be below " + std::to_string(fIt->body.size()));
		}

		bIt = fIt->body.cbegin() + idx;
		iIt = bIt->cbegin();
	}

	bool readNext(Program::Instruction& isn)
	{
		while(iIt == bIt->cend())
		{
			bIt++;

			if(bIt == fIt->body.cend())
			{
				return false;
			}

			iIt = bIt->cbegin();
		}

		isn = *iIt++;
		return true;
	}
};

static inline std::vector<uint32_t> run(Reader &r, const Program::FrameInfo &fInfo, uint32_t* args)
{
	std::unique_ptr<uint32_t[]> frame(new uint32_t[fInfo.nArgs + fInfo.nLocals + fInfo.maxStack]);

	auto state = State
	{
		.fp = frame.get() + fInfo.nArgs,
		.sp = frame.get() + fInfo.nArgs + fInfo.nLocals
	};

	std::copy(args, args + fInfo.nArgs, frame.get());

	for(Program::Instruction isn; r.readNext(isn);)
	{
		switch(isn.group)
		{
			case Program::OperationGroup::Immediate:
				push(fInfo, state, isn.value);
				break;

			case Program::OperationGroup::Duplicate:
			{
				const auto tos = pop(fInfo, state);
				push(fInfo, state, tos);
				push(fInfo, state, tos);
				break;
			}

			case Program::OperationGroup::LoadStore:
				switch(isn.lsOp)
				{
					case Program::LoadStoreOperation::LoadLocal:
						push(fInfo, state, loadLocal(fInfo, state, isn.varIdx));
						break;
					case Program::LoadStoreOperation::LoadArgument:
						push(fInfo, state, loadArg(fInfo, state, isn.varIdx));
						break;
					case Program::LoadStoreOperation::StoreLocal:
						storeLocal(fInfo, state, isn.varIdx, pop(fInfo, state));
						break;
					case Program::LoadStoreOperation::StoreArgument:
						storeArg(fInfo, state, isn.varIdx, pop(fInfo, state));
						break;
					default: throw std::runtime_error("Unknown memory operation");
				}
				break;

			case Program::OperationGroup::Binary:
			{
				const auto arg2 = pop(fInfo, state);
				const auto arg1 = pop(fInfo, state);

				switch(isn.binOp)
				{
					case Program::BinaryOperation::Add: push(fInfo, state, arg1 + arg2); break;
					case Program::BinaryOperation::Sub: push(fInfo, state, arg1 - arg2); break;
					case Program::BinaryOperation::Mul: push(fInfo, state, arg1 * arg2); break;
					case Program::BinaryOperation::Div: push(fInfo, state, arg1 / arg2); break;
					case Program::BinaryOperation::Mod: push(fInfo, state, arg1 % arg2); break;
					case Program::BinaryOperation::And: push(fInfo, state, arg1 & arg2); break;
					case Program::BinaryOperation::Ior: push(fInfo, state, arg1 | arg2); break;
					case Program::BinaryOperation::Xor: push(fInfo, state, arg1 ^ arg2); break;
					case Program::BinaryOperation::Lsh: push(fInfo, state, arg1 << arg2); break;
					case Program::BinaryOperation::Rsh: push(fInfo, state, arg1 >> arg2); break;
					default: throw std::runtime_error("Unknown binary operation");
				}
				break;
			}

			case Program::OperationGroup::Conditional:
			{
				const auto arg2 = pop(fInfo, state);
				const auto arg1 = pop(fInfo, state);

				bool doIt = false;

				switch(isn.cond)
				{
					case Program::BranchCondition::IfEqual:      doIt = arg1 == arg2; break;
					case Program::BranchCondition::IfGreater:    doIt = arg1 > arg2; break;
					case Program::BranchCondition::IfLess:       doIt = arg1 < arg2; break;
					case Program::BranchCondition::IfNotEqual:   doIt = arg1 != arg2; break;
					case Program::BranchCondition::IfNotGreater: doIt = arg1 <= arg2; break;
					case Program::BranchCondition::IfNotLess:    doIt = arg1 >= arg2; break;
					default: throw std::runtime_error("Unknown branch condition");
				}

				if(doIt)
				{
					r.seekBlock(isn.targetBlockIdx);
				}

				break;
			}

			case Program::OperationGroup::Call:
			{
				auto idx = pop(fInfo, state);

				Program::FrameInfo nfInfo;
				auto rp = r.openFunction(0, nfInfo);

				if(const auto nActArgs = state.sp - (state.fp + fInfo.nLocals); nActArgs < nfInfo.nArgs)
				{
					throw std::runtime_error("Function called with insufficient number of arguments: " + std::to_string(nActArgs) + " (should be " + std::to_string(nfInfo.nArgs) + ")");
				}

				auto ret = run(r, nfInfo, state.sp - nfInfo.nArgs);

				state.sp -= fInfo.nArgs;
				for(const auto v: ret)
				{
					push(fInfo, state, v);
				}

				r.restore(rp);
				break;
			}

			case Program::OperationGroup::Return:
			{
				return {state.fp + fInfo.nLocals, state.sp};
			}

			case Program::OperationGroup::Jump:
				r.seekBlock(isn.targetBlockIdx);
				break;

			default:
				throw std::runtime_error("Unknown instruction group");
		}
	}

	throw std::runtime_error("No return at end of function");
}

std::vector<uint32_t> interpret(const Program& p, const std::vector<uint32_t> &args, size_t stackSize)
{
	uint32_t stack[stackSize];

	Program::FrameInfo fi;
	Reader r(p, fi);
	std::copy(args.begin(), args.end(), stack);
	return ::run(r, p.functions[0].info, stack);
}
