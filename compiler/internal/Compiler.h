#ifndef COMPILER_COMPILE_H_
#define COMPILER_COMPILE_H_

#include "compiler/ast/ProgramObjectSet.h"

#include "compiler/ast/Function.h"
#include "compiler/ir/Function.h"

#include "program/Program.h"

namespace comp {

enum class Options
{
    doJumpOptimizations = 0x00000001,
    propagateConstants  = 0x00000002,
    eliminateDeadCode   = 0x00000004,
};

static constexpr inline Options operator| (Options x, Options y)
{
    typedef std::underlying_type<Options>::type T;
    return Options(T(x) | T(y));
}

static constexpr inline bool operator& (Options x, Options y)
{
    typedef std::underlying_type<Options>::type T;
    return T(x) & T(y);
}

class Compiler
{
	std::shared_ptr<ast::Function> entryPoint;
	ast::ProgramObjectSet gi;

public:

private:
	static std::shared_ptr<ir::Function> generateIr(std::shared_ptr<ast::Function> f);
	static void optimizeIr(std::shared_ptr<ir::Function> f, Options opt);
	static bool removeEmptyBasicBlocks(std::shared_ptr<ir::Function> f);
	static bool mergeBasicBlocks(std::shared_ptr<ir::Function> f);
	static bool propagateConstants(std::shared_ptr<ir::Function> f);
	static bool eliminateDeadCode(std::shared_ptr<ir::Function> f);

	static inline constexpr auto defaultFlags =
			Options::doJumpOptimizations |
			Options::propagateConstants |
			Options::eliminateDeadCode;
public:
	inline Compiler(std::shared_ptr<ast::Function> entryPoint):
		entryPoint(entryPoint),
		gi(ast::ProgramObjectSet::shakeTree(entryPoint)) {}

	std::string dumpAst();
	std::string dumpCfg(Options opt = defaultFlags);

	prog::Program compile(); // TBD
};

} // namespace comp

#endif /* COMPILER_COMPILE_H_ */
