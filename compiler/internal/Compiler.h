#ifndef COMPILER_COMPILE_H_
#define COMPILER_COMPILE_H_

#include "compiler/ast/ProgramObjectSet.h"

#include "compiler/ast/Function.h"
#include "compiler/ir/Function.h"

#include "program/Program.h"

namespace comp {

class Compiler
{
	std::shared_ptr<ast::Function> entryPoint;
	ast::ProgramObjectSet gi;

	static std::shared_ptr<ir::Function> generateIr(std::shared_ptr<ast::Function> f);

public:
	inline Compiler(std::shared_ptr<ast::Function> entryPoint):
		entryPoint(entryPoint),
		gi(ast::ProgramObjectSet::shakeTree(entryPoint)) {}

	std::string dumpAst();
	std::string dumpCfg();

	prog::Program compile(); // TBD
};

} // namespace comp

#endif /* COMPILER_COMPILE_H_ */
