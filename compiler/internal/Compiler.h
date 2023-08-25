#ifndef COMPILER_COMPILE_H_
#define COMPILER_COMPILE_H_

#include "compiler/model/Function.h"

#include "GlobalIdentifiers.h"

#include "program/Program.h"

namespace comp {

class Compiler
{
	std::shared_ptr<Function> entryPoint;
	GlobalIdentifiers gi;

public:
	inline Compiler(std::shared_ptr<Function> entryPoint):
		entryPoint(entryPoint),
		gi(GlobalIdentifiers::gather(entryPoint)) {}

	std::string dumpAst();
	prog::Program compile();
};


} // namespace comp

#endif /* COMPILER_COMPILE_H_ */
