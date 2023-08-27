#ifndef COMPILER_REFERENCEEXRACTION_H_
#define COMPILER_REFERENCEEXRACTION_H_

#include "compiler/ast/Class.h"
#include "compiler/ast/Function.h"

#include <vector>

namespace comp {
namespace ast {

struct ProgramObjectSet
{
	std::vector<std::shared_ptr<Function>> functions;
	std::vector<std::shared_ptr<Class>> classes;

	static ProgramObjectSet shakeTree(std::shared_ptr<Function> entryPoint);

	size_t getClassIndex(const Class*) const;
	size_t getFunctionIndex(const Function*) const;
};

} // namespace ast
} // namespace comp

#endif /* COMPILER_REFERENCEEXRACTION_H_ */
