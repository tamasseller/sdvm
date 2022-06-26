#ifndef JIT_BYTECODE_H_
#define JIT_BYTECODE_H_

#include <cstdint>

struct Bytecode
{
	struct FunctionInfo
	{
		uint32_t nLabels, nRet;
		bool hasNonTailCall;
	};

	struct Instruction
	{
		enum class OperationGroup
		{
			Immediate,
			Binary,
			Conditional,
			Jump,
			Label,
			Move,
			Call,
			Ret
		};

		enum class BinaryOperation
		{
			Add, Sub, Mul, Div, Mod, And, Ior, Xor, Lsh, Rsh, Ash
		};

		enum class Condition
		{
			Equal, NotEqual,
			UnsignedGreater, UnsignedNotGreater,
			UnsignedLess, UnsignedNotLess,
			SignedGreater, SignedNotGreater,
			SignedLess, SignedNotLess
		};

		OperationGroup g;

		enum class DupDirection
		{
			In, Out
		};

		enum class DupTarget
		{
			Arg, Stack
		};

		union
		{
			struct Immediate
			{
				uint32_t value;
			} imm;

			struct Binary
			{
				BinaryOperation op;
			} bin;

			struct Conditional
			{
				Condition cond;
				uint32_t targetIdx;
			} cond;

			struct Jump
			{
				uint32_t targetIdx;
			} jump;

			struct Label
			{
				uint32_t stackAdjustment;
			} label;

			struct Move
			{
				DupDirection dir;
				DupTarget target;
				uint32_t idx;
			} move;

			struct Call
			{
				uint32_t nArgs, nRet;
			} call;

			struct Return
			{
				uint32_t nRet;
			} ret;
		};
	};

	struct InstructionStreamReader
	{

	};
};

#endif /* JIT_BYTECODE_H_ */
