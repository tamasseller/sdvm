#ifndef JIT_COMPILER_H_
#define JIT_COMPILER_H_

#include "Bytecode.h"

#include <cstddef>

class Compiler
{
	struct Output
	{
		uint16_t *start;
		size_t length;
	};

	uint16_t *compile(uint16_t fnIdx, const Output& out, const Bytecode::FunctionInfo& info, Bytecode::InstructionStreamReader& reader);
};

#endif /* JIT_COMPILER_H_ */
