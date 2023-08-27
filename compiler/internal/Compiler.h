#ifndef COMPILER_COMPILE_H_
#define COMPILER_COMPILE_H_

#include "compiler/ast/Function.h"
#include "compiler/ast/ProgramObjectSet.h"

#include "program/Program.h"

namespace comp {

class Compiler
{
	std::shared_ptr<ast::Function> entryPoint;
	ast::ProgramObjectSet gi;

public:
	inline Compiler(std::shared_ptr<ast::Function> entryPoint):
		entryPoint(entryPoint),
		gi(ast::ProgramObjectSet::shakeTree(entryPoint)) {}

	std::string dumpAst();
	std::string dumpTac();
	prog::Program compile();
};

} // namespace comp

#endif /* COMPILER_COMPILE_H_ */
