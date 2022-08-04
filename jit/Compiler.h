#ifndef JIT_COMPILER_H_
#define JIT_COMPILER_H_

#include "Bytecode.h"

#include <cstddef>

struct Compiler
{
	struct Output
	{
		uint16_t *start;
		size_t length;
	};

	static uint16_t *compile(uint16_t fnIdx, const Output& out, Bytecode::FunctionReader *reader);
};

#endif /* JIT_COMPILER_H_ */
