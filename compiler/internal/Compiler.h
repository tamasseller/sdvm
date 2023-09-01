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
	static bool removeEmptyBasicBlocks(std::shared_ptr<ir::Function> f);
	static bool mergeBasicBlocks(std::shared_ptr<ir::Function> f);

public:
	enum class Options
	{
	    doJumpOptimizations = 0x00000001,
	};

	inline Compiler(std::shared_ptr<ast::Function> entryPoint):
		entryPoint(entryPoint),
		gi(ast::ProgramObjectSet::shakeTree(entryPoint)) {}

	std::string dumpAst();
	std::string dumpCfg(Compiler::Options opt = Compiler::Options(0));

	prog::Program compile(); // TBD
};

static inline Compiler::Options operator| (Compiler::Options x, Compiler::Options y)
{
    typedef std::underlying_type<Compiler::Options>::type T;
    return Compiler::Options(T(x) | T(y));
}

static inline bool operator& (Compiler::Options x, Compiler::Options y)
{
    typedef std::underlying_type<Compiler::Options>::type T;
    return T(x) & T(y);
}

} // namespace comp

#endif /* COMPILER_COMPILE_H_ */
