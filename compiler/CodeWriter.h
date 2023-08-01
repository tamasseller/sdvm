#ifndef COMPILER_CODEWRITER_H_
#define COMPILER_CODEWRITER_H_

#include "program/Instruction.h"

#include <vector>

namespace comp {

struct CodeWriter
{
	int stackDepth = 0, maxStackDepth = 0;
	bool hasCall = false;
	std::vector<prog::Instruction> code;

	void write(prog::Instruction isn);
};

} //namespace comp

#endif /* COMPILER_CODEWRITER_H_ */
