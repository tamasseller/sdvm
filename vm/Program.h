#ifndef MODEL_PROGRAM_H_
#define MODEL_PROGRAM_H_

#include <vector>

#include <cstdint>

struct Program
{
	enum class Operation
	{
		Immediate,
		Duplicate,
		Load, Store,
		Add, Sub, Mul, Div, Mod,
		And, Or, Xor, Lshift, Rshift,
		SkipIfTrue, Jump, Call
	};

	struct Instruction
	{
		Operation op;
		uint32_t arg;
	};

	using Block = std::vector<Instruction>;
	using Body = std::vector<Block>;

	struct Function
	{
		uint32_t maxStack, nLocals, nArgs;
		Body body;
	};

	std::vector<Function> functions;
};

#endif /* MODEL_PROGRAM_H_ */

