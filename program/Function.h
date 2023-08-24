#ifndef PROGRAM_FUNCTION_H_
#define PROGRAM_FUNCTION_H_

#include "Instruction.h"

#include <vector>

namespace prog {
struct Function

{
	size_t nRefs, nScalars;
	std::vector<Instruction> code;
};

} //namespace prog

#endif /* PROGRAM_FUNCTION_H_ */
