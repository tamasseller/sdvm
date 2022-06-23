#ifndef JIT_BYTECODE_H_
#define JIT_BYTECODE_H_

#include <cstdint>

struct Bytecode
{
	enum class OperationGroup
	{
		Binary,
		Conditional,
		Jump,
		Immediate,
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

	union
	{
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

		struct Immediate
		{
			uint32_t value;
		} imm;
	};
};

#endif /* JIT_BYTECODE_H_ */
