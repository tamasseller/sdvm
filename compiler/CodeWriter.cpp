#include "CodeWriter.h"

#include "assert.h"

using namespace comp;

void CodeWriter::write(prog::Instruction isn)
{
	hasCall = hasCall || isn.op == prog::Instruction::Operation::Call || isn.op == prog::Instruction::Operation::CallV;
	stackDepth += isn.stackBalance();
	assert(0 <= stackDepth);
	maxStackDepth = std::max(maxStackDepth, stackDepth);
	code.push_back(isn);
}
