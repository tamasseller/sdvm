#ifndef PROGRAM_H_
#define PROGRAM_H_

#include <optional>

#include "Function.h"

namespace prog {

struct Program
{
	const obj::Type* staticType;
	std::vector<Function> functions;
};

} //namespace prog

#endif /* PROGRAM_H_ */
