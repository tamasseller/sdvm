#ifndef COMPILER_RVALUE_H_
#define COMPILER_RVALUE_H_

#include "ValueType.h"

#include "program/Instruction.h"

#include <functional>
#include <vector>

namespace comp {

class CodeWriter;

struct RValue {
	ValueType type;
	std::function<void(CodeWriter&)> manifest;
};

} //namespace comp

#endif /* COMPILER_RVALUE_H_ */
