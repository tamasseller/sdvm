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


static inline std::vector<uint32_t> run(const Program& p, const Program::Function& f, uint32_t* args)
{
	std::unique_ptr<uint32_t[]> frame(new uint32_t[f.info.nArgs + f.info.nLocals + f.info.maxStack]);

	auto state = State
	{
		.fp = frame.get() + f.info.nArgs,
		.sp = frame.get() + f.info.nArgs + f.info.nLocals
	};

	std::copy(args, args + f.info.nArgs, frame.get());

	for(auto blockIt = f.body.cbegin(); blockIt != f.body.cend();)
	{
		auto next = blockIt + 1;
		bool done = false;

		for(auto iit = blockIt->cbegin(); !done && iit != blockIt->cend(); ++iit)
		{
			switch(iit->group)
			{
				case Program::OperationGroup::Immediate:
					push(f.info, state, iit->value);
					break;

				case Program::OperationGroup::Duplicate:
				{
					const auto tos = pop(f.info, state);
					push(f.info, state, tos);
					push(f.info, state, tos);
					break;
				}

				case Program::OperationGroup::LoadStore:
					switch(iit->lsOp)
					{
						case Program::LoadStoreOperation::LoadLocal:
							push(f.info, state, loadLocal(f.info, state, iit->varIdx));
							break;
						case Program::LoadStoreOperation::LoadArgument:
							push(f.info, state, loadArg(f.info, state, iit->varIdx));
							break;
						case Program::LoadStoreOperation::StoreLocal:
							storeLocal(f.info, state, iit->varIdx, pop(f.info, state));
							break;
						case Program::LoadStoreOperation::StoreArgument:
							storeArg(f.info, state, iit->varIdx, pop(f.info, state));
							break;
						default: throw std::runtime_error("Unknown memory operation");
					}
					break;

				case Program::OperationGroup::Binary:
				{
					const auto arg2 = pop(f.info, state);
					const auto arg1 = pop(f.info, state);

					switch(iit->binOp)
					{
						case Program::BinaryOperation::Add: push(f.info, state, arg1 + arg2); break;
						case Program::BinaryOperation::Sub: push(f.info, state, arg1 - arg2); break;
						case Program::BinaryOperation::Mul: push(f.info, state, arg1 * arg2); break;
						case Program::BinaryOperation::Div: push(f.info, state, arg1 / arg2); break;
						case Program::BinaryOperation::Mod: push(f.info, state, arg1 % arg2); break;
						case Program::BinaryOperation::And: push(f.info, state, arg1 & arg2); break;
						case Program::BinaryOperation::Ior: push(f.info, state, arg1 | arg2); break;
						case Program::BinaryOperation::Xor: push(f.info, state, arg1 ^ arg2); break;
						case Program::BinaryOperation::Lsh: push(f.info, state, arg1 << arg2); break;
						case Program::BinaryOperation::Rsh: push(f.info, state, arg1 >> arg2); break;
						default: throw std::runtime_error("Unknown binary operation");
					}
					break;
				}

				case Program::OperationGroup::Conditional:
				{
					const auto arg2 = pop(f.info, state);
					const auto arg1 = pop(f.info, state);

					bool doIt = false;

					switch(iit->cond)
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
						if(f.body.size() <= iit->targetBlockIdx)
						{
							throw std::runtime_error("Jump to unknown block: " + std::to_string(iit->targetBlockIdx) + " (should be below " + std::to_string(blockIt->size()) + ")");
						}

						next = f.body.cbegin() + iit->targetBlockIdx;
						done = true;
					}

					break;
				}

				case Program::OperationGroup::Call:
				{
					auto idx = pop(f.info, state);

					if(p.functions.size() <= idx)
					{
						throw std::runtime_error("Unknown function called: " + std::to_string(idx) + " (should be below " + std::to_string(p.functions.size()));
					}

					const auto &nf = p.functions[idx];

					if(const auto nActArgs = state.sp - (state.fp + f.info.nLocals); nActArgs < nf.info.nArgs)
					{
						throw std::runtime_error("Function called with insufficient number of arguments: " + std::to_string(nActArgs) + " (should be " + std::to_string(nf.info.nArgs) + ")");
					}

					auto ret = run(p, nf, state.sp - nf.info.nArgs);

					state.sp -= f.info.nArgs;
					for(const auto v: ret)
					{
						push(f.info, state, v);
					}

					break;
				}

				case Program::OperationGroup::Jump:
					if(blockIt->size() <= iit->targetBlockIdx)
					{
						throw std::runtime_error("Jump to unknown block: " + std::to_string(iit->targetBlockIdx) + " (should be below " + std::to_string(blockIt->size()));
					}

					next = f.body.cbegin() + iit->targetBlockIdx;
					done = true;
					break;

				default:
					throw std::runtime_error("Unknown instruction group");
			}
		}

		blockIt = next;
	}

	return {state.fp + f.info.nLocals, state.sp};
}

std::vector<uint32_t> Program::interpret(const std::vector<uint32_t> &args, size_t stackSize) const
{
	uint32_t stack[stackSize];
	std::copy(args.begin(), args.end(), stack);
	return ::run(*this, functions[0], stack);
}
