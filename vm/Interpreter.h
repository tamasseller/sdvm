#ifndef VM_INTERPRETER_H_
#define VM_INTERPRETER_H_

#include <memory>
#include <vector>
#include "Visa.h"

#define X_INTERPRETER_ERRORS()																			\
		X(Underflow, "OpStack underflow")                                                               \
		X(Overflow, "OpStack overflow")                                                                 \
		X(LocaLoadOob, "Local load out of bounds (%d should be below %d)")                              \
		X(LocaStoreOob, "Local store out of bounds (%d should be below %d)")                            \
		X(ArgumentLoadOob, "Argument load out of bounds (%d should be below %d)")                       \
		X(ArgumentStoreOob, "Argument store out of bounds (%d should be below %d)")                     \
		X(UnknownOperation, "Unknown operation")                                                        \
		X(NotEnoughArguments,"Function called with insufficient number of arguments (%d should be %d)") \
		X(WrongReturnValues,"Function returns with wrong number of values (%d should be %d)") 			\
		X(NoReturnAtEnd, "No return at end of function")

enum class InterpreterError
{
#define X(sym, desc) sym,
	X_INTERPRETER_ERRORS()
#undef X
};

template<class ErrorHandler>
class Interpreter
{
	static constexpr auto frameHeaderWords = 4;

	struct State
	{
		uint32_t *fp, *sp;

		inline uint32_t pop(const Visa::FrameInfo& info)
		{
			if(sp <= fp + info.nLocals)
			{
				ErrorHandler::take(InterpreterError::Underflow);
				return 0;
			}

			return *--sp;
		}

		inline void push(const Visa::FrameInfo& info, uint32_t v)
		{
			if(fp + info.nLocals + info.maxStack <= sp)
			{
				ErrorHandler::take(InterpreterError::Overflow);
			}
			else
			{
				*sp++ = v;
			}
		}

		inline uint32_t loadLocal(const Visa::FrameInfo& info, uint32_t idx)
		{
			if(info.nLocals <= idx)
			{
				ErrorHandler::take(InterpreterError::LocaLoadOob, idx, info.nLocals);
				return 0;
			}

			return fp[idx];
		}

		inline void storeLocal(const Visa::FrameInfo& info, uint32_t idx, uint32_t v)
		{
			if(info.nLocals <= idx)
			{
				ErrorHandler::take(InterpreterError::LocaStoreOob, idx, info.nLocals);
			}
			else
			{
				fp[idx] = v;
			}
		}

		inline uint32_t loadArg(const Visa::FrameInfo& info, uint32_t idx)
		{
			if(info.nArgs <= idx)
			{
				ErrorHandler::take(InterpreterError::ArgumentLoadOob, idx, info.nArgs);
				return 0;
			}

			return fp[(int32_t)(-(idx + frameHeaderWords + 1))];
		}

		inline void storeArg(const Visa::FrameInfo& info, uint32_t idx, uint32_t v)
		{
			if(info.nArgs <= idx)
			{
				ErrorHandler::take(InterpreterError::ArgumentStoreOob, idx, info.nArgs);
			}
			else
			{
				fp[(int32_t)(-(idx + frameHeaderWords + 1))] = v;

			}
		}
	};

public:
	template<class ProgramReader>
	static std::vector<uint32_t> interpret(ProgramReader& r, const std::vector<uint32_t> &args, size_t stackSize = 4096)
	{
		static constexpr uint32_t dummyFrameHeader[] = {-1u, -1u, -1u, -1u};
		static_assert(frameHeaderWords == sizeof(dummyFrameHeader) / sizeof(dummyFrameHeader[0]));

		Visa::FrameInfo fInfo;
		r.init(fInfo);

		uint32_t stack[stackSize];
		std::copy(args.rbegin(), args.rend(), stack);
		std::copy(dummyFrameHeader, dummyFrameHeader + frameHeaderWords, stack + args.size());

		State state;
		state.fp = stack + fInfo.nArgs + frameHeaderWords;
		state.sp = state.fp + fInfo.nLocals;

		for(Visa::Instruction isn; r.readNext(isn);)
		{
			switch(isn.group)
			{
				case Visa::OperationGroup::Immediate:
					state.push(fInfo, isn.value);
					break;

				case Visa::OperationGroup::Duplicate:
				{
					const auto tos = state.pop(fInfo);
					state.push(fInfo, tos);
					state.push(fInfo, tos);
					break;
				}

				case Visa::OperationGroup::Load:
				{
					uint32_t v;

					switch(isn.dest)
					{
						case Visa::LoadStoreDestination::Local:
							v = state.loadLocal(fInfo, isn.varIdx);
							break;
						case Visa::LoadStoreDestination::Argument:
							v = state.loadArg(fInfo, isn.varIdx);
							break;

						default: ErrorHandler::take(InterpreterError::UnknownOperation);
					}

					state.push(fInfo, v);
					break;
				}

				case Visa::OperationGroup::Store:
				{
					const uint32_t v = state.pop(fInfo);

					switch(isn.dest)
					{
						case Visa::LoadStoreDestination::Local:
							state.storeLocal(fInfo, isn.varIdx, v);
							break;
						case Visa::LoadStoreDestination::Argument:
							state.storeArg(fInfo, isn.varIdx, v);
							break;

						default: ErrorHandler::take(InterpreterError::UnknownOperation);
					}
					break;
				}

				case Visa::OperationGroup::Binary:
				{
					const auto arg2 = state.pop(fInfo);
					const auto arg1 = state.pop(fInfo);

					switch(isn.binOp)
					{
						case Visa::BinaryOperation::Add:        state.push(fInfo, arg1 + arg2);  break;
						case Visa::BinaryOperation::Sub:        state.push(fInfo, arg1 - arg2);  break;
						case Visa::BinaryOperation::Mul:        state.push(fInfo, arg1 * arg2);  break;
						case Visa::BinaryOperation::Div:        state.push(fInfo, arg1 / arg2);  break;
						case Visa::BinaryOperation::Mod:        state.push(fInfo, arg1 % arg2);  break;
						case Visa::BinaryOperation::And:        state.push(fInfo, arg1 & arg2);  break;
						case Visa::BinaryOperation::Ior:        state.push(fInfo, arg1 | arg2);  break;
						case Visa::BinaryOperation::Xor:        state.push(fInfo, arg1 ^ arg2);  break;
						case Visa::BinaryOperation::Lsh:        state.push(fInfo, arg1 << arg2); break;
						case Visa::BinaryOperation::Rsh:        state.push(fInfo, arg1 >> arg2); break;
						case Visa::BinaryOperation::Equal:      state.push(fInfo, arg1 == arg2); break;
						case Visa::BinaryOperation::Less:       state.push(fInfo, arg1 < arg2);  break;
						case Visa::BinaryOperation::Greater:    state.push(fInfo, arg1 > arg2);  break;
						case Visa::BinaryOperation::NotEqual:   state.push(fInfo, arg1 != arg2); break;
						case Visa::BinaryOperation::NotLess:    state.push(fInfo, arg1 >= arg2); break;
						case Visa::BinaryOperation::NotGreater: state.push(fInfo, arg1 <= arg2); break;
						case Visa::BinaryOperation::Both:       state.push(fInfo, arg1 && arg2); break;
						case Visa::BinaryOperation::Either:     state.push(fInfo, arg1 || arg2); break;

						default: ErrorHandler::take(InterpreterError::UnknownOperation);
					}
					break;
				}

				case Visa::OperationGroup::Conditional:
					if(!state.pop(fInfo))
					{
						break;
					}
					// NO BREAK
				case Visa::OperationGroup::Jump:
					r.seekBlock(isn.targetBlockIdx);
					break;

				case Visa::OperationGroup::Call:
				{
					Visa::FrameInfo nfInfo;
					auto rp = r.openFunction(state.pop(fInfo), nfInfo);

					if(const auto nActArgs = state.sp - (state.fp + fInfo.nLocals); nActArgs < nfInfo.nArgs)
					{
						ErrorHandler::take(InterpreterError::NotEnoughArguments, nActArgs, nfInfo.nArgs);
						Visa::FrameInfo _;
						r.restore(rp, _);
					}
					else
					{
						fInfo = nfInfo;

						*state.sp++ = rp.bIdx;
						*state.sp++ = rp.fIdx;
						*state.sp++ = rp.iIdx;
						*state.sp++ = (uint32_t)(state.fp - stack);

						state.fp = state.sp;
						state.sp += fInfo.nLocals;
					}

					break;
				}

				case Visa::OperationGroup::Return:
				{
					auto stackStart = state.fp + fInfo.nLocals;
					auto stackEnd = state.sp;

					if(const auto nActRet = stackEnd - stackStart; nActRet != fInfo.nRet)
					{
						ErrorHandler::take(InterpreterError::WrongReturnValues, nActRet, fInfo.nRet);
					}

					if(auto oFp = state.fp[-1]; 0 < (int)oFp)
					{
						auto oTos = state.fp - frameHeaderWords - fInfo.nArgs;
						std::copy(stackStart, stackEnd, oTos);

						decltype(r.openFunction(0, *((Visa::FrameInfo*)nullptr))) rp;
						rp.iIdx = state.fp[-2];
						rp.fIdx = state.fp[-3];
						rp.bIdx = state.fp[-4];

						Visa::FrameInfo ofInfo;
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
			default:

				ErrorHandler::take(InterpreterError::UnknownOperation);
			}
		}

		ErrorHandler::take(InterpreterError::NoReturnAtEnd);
		return {};
	}
};

#endif /* VM_INTERPRETER_H_ */
