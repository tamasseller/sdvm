#ifndef VM_INTERPRETER_H_
#define VM_INTERPRETER_H_

#include "Program.h"

#include <stdexcept>
#include <memory>

class Interpreter
{
	static constexpr auto frameHeaderWords = 4;

	struct State
	{
		uint32_t *fp, *sp;

		inline uint32_t pop(const Program::FrameInfo& info)
		{
			if(sp <= fp + info.nLocals)
			{
				throw std::runtime_error("OpStack underflow");
			}

			return *--sp;
		}

		inline void push(const Program::FrameInfo& info, uint32_t v)
		{
			if(fp + info.nLocals + info.maxStack <= sp)
			{
				throw std::runtime_error("OpStack overflow");
			}

			*sp++ = v;
		}

		inline uint32_t loadLocal(const Program::FrameInfo& info, uint32_t idx)
		{
			if(info.nLocals <= idx)
			{
				throw std::runtime_error("Local load out of bounds (" + std::to_string(idx) + " should be below " + std::to_string(info.nLocals) + ")");
			}

			return fp[idx];
		}

		inline void storeLocal(const Program::FrameInfo& info, uint32_t idx, uint32_t v)
		{
			if(info.nLocals <= idx)
			{
				throw std::runtime_error("Local store out of bounds (" + std::to_string(idx) + " should be below " + std::to_string(info.nLocals) + ")");
			}

			fp[idx] = v;
		}

		inline uint32_t loadArg(const Program::FrameInfo& info, uint32_t idx)
		{
			if(info.nArgs <= idx)
			{
				throw std::runtime_error("Argument load out of bounds (" + std::to_string(idx) + " should be below " + std::to_string(info.nArgs) + ")");
			}

			return fp[(int32_t)(-(idx + frameHeaderWords + 1))];
		}

		inline void storeArg(const Program::FrameInfo& info, uint32_t idx, uint32_t v)
		{
			if(info.nArgs <= idx)
			{
				throw std::runtime_error("Argument store out of bounds (" + std::to_string(idx) + " should be below " + std::to_string(info.nArgs) + ")");
			}

			fp[(int32_t)(-(idx + frameHeaderWords + 1))] = v;
		}
	};

public:
	template<class ProgramReader>
	static std::vector<uint32_t> interpret(ProgramReader& r, const std::vector<uint32_t> &args, size_t stackSize = 4096)
	{
		static constexpr uint32_t dummyFrameHeader[] = {-1u, -1u, -1u, -1u};
		static_assert(frameHeaderWords == sizeof(dummyFrameHeader) / sizeof(dummyFrameHeader[0]));

		Program::FrameInfo fInfo;
		r.init(fInfo);

		uint32_t stack[stackSize];
		std::copy(args.rbegin(), args.rend(), stack);
		std::copy(dummyFrameHeader, dummyFrameHeader + frameHeaderWords, stack + args.size());

		State state;
		state.fp = stack + fInfo.nArgs + frameHeaderWords;
		state.sp = state.fp + fInfo.nLocals;

		for(Program::Instruction isn; r.readNext(isn);)
		{
			switch(isn.group)
			{
				case Program::OperationGroup::Immediate:
					state.push(fInfo, isn.value);
					break;

				case Program::OperationGroup::Duplicate:
				{
					const auto tos = state.pop(fInfo);
					state.push(fInfo, tos);
					state.push(fInfo, tos);
					break;
				}

				case Program::OperationGroup::LoadStore:
					switch(isn.lsOp)
					{
						case Program::LoadStoreOperation::LoadLocal:
							state.push(fInfo, state.loadLocal(fInfo, isn.varIdx));
							break;
						case Program::LoadStoreOperation::LoadArgument:
							state.push(fInfo, state.loadArg(fInfo, isn.varIdx));
							break;
						case Program::LoadStoreOperation::StoreLocal:
							state.storeLocal(fInfo, isn.varIdx, state.pop(fInfo));
							break;
						case Program::LoadStoreOperation::StoreArgument:
							state.storeArg(fInfo, isn.varIdx, state.pop(fInfo));
							break;
						default: throw std::runtime_error("Unknown memory operation");
					}
					break;

				case Program::OperationGroup::Binary:
				{
					const auto arg2 = state.pop(fInfo);
					const auto arg1 = state.pop(fInfo);

					switch(isn.binOp)
					{
						case Program::BinaryOperation::Add: state.push(fInfo, arg1 + arg2); break;
						case Program::BinaryOperation::Sub: state.push(fInfo, arg1 - arg2); break;
						case Program::BinaryOperation::Mul: state.push(fInfo, arg1 * arg2); break;
						case Program::BinaryOperation::Div: state.push(fInfo, arg1 / arg2); break;
						case Program::BinaryOperation::Mod: state.push(fInfo, arg1 % arg2); break;
						case Program::BinaryOperation::And: state.push(fInfo, arg1 & arg2); break;
						case Program::BinaryOperation::Ior: state.push(fInfo, arg1 | arg2); break;
						case Program::BinaryOperation::Xor: state.push(fInfo, arg1 ^ arg2); break;
						case Program::BinaryOperation::Lsh: state.push(fInfo, arg1 << arg2); break;
						case Program::BinaryOperation::Rsh: state.push(fInfo, arg1 >> arg2); break;
						default: throw std::runtime_error("Unknown binary operation");
					}
					break;
				}

				case Program::OperationGroup::Conditional:
				{
					const auto arg2 = state.pop(fInfo);
					const auto arg1 = state.pop(fInfo);

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
					Program::FrameInfo nfInfo;
					auto rp = r.openFunction(state.pop(fInfo), nfInfo);

					if(const auto nActArgs = state.sp - (state.fp + fInfo.nLocals); nActArgs < nfInfo.nArgs)
					{
						throw std::runtime_error("Function called with insufficient number of arguments: " + std::to_string(nActArgs) + " (should be " + std::to_string(nfInfo.nArgs) + ")");
					}

					fInfo = nfInfo;

					*state.sp++ = rp.bIdx;
					*state.sp++ = rp.fIdx;
					*state.sp++ = rp.iIdx;
					*state.sp++ = (uint32_t)(state.fp - stack);

					state.fp = state.sp;
					state.sp += fInfo.nLocals;

					break;
				}

				case Program::OperationGroup::Return:
				{
					auto stackStart = state.fp + fInfo.nLocals;
					auto stackEnd = state.sp;

					if(auto oFp = state.fp[-1]; 0 < (int)oFp)
					{
						auto oTos = state.fp - frameHeaderWords - fInfo.nArgs;
						std::copy(stackStart, stackEnd, oTos);

						decltype(r.openFunction(0, *((Program::FrameInfo*)nullptr))) rp;
						rp.iIdx = state.fp[-2];
						rp.fIdx = state.fp[-3];
						rp.bIdx = state.fp[-4];

						Program::FrameInfo ofInfo;
						r.restore(rp, ofInfo);

						state.fp = stack + oFp;
						state.sp = oTos + (stackEnd - stackStart);
						break;
					}
					else
					{
						return {stackStart, stackEnd};
					}
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
};

#endif /* VM_INTERPRETER_H_ */
