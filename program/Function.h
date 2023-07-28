#ifndef PROGRAM_FUNCTION_H_
#define PROGRAM_FUNCTION_H_

#include "Frame.h"
#include "Instruction.h"

namespace prog {

struct Function
{
	Frame frame;
	std::vector<Instruction> code;
};

} //namespace prog

#endif /* PROGRAM_FUNCTION_H_ */
