#ifndef PROGRAM_H_
#define PROGRAM_H_

#include <optional>

#include "TypeInfo.h"
#include "Function.h"

namespace prog {

struct Program
{
	// First entry is the global object
	std::vector<TypeInfo> types;

	// First entry is the entry point
	std::vector<Function> functions;
};

} //namespace prog

#endif /* PROGRAM_H_ */
