#ifndef COMPILER_IR_FUNCTION_H_
#define COMPILER_IR_FUNCTION_H_

#include "BasicBlock.h"

#include "compiler/ast/Function.h"

#include <functional>

namespace comp {
namespace ir {

struct Function
{
	std::vector<std::shared_ptr<Temporary>> args;
	std::shared_ptr<BasicBlock> entry;

	inline Function(decltype(args) args, decltype(entry) entry): args(args), entry(entry) {}

	std::string dump(ast::ProgramObjectSet& gi) const;

	void traverse(std::function<void(std::shared_ptr<BasicBlock>)> c) const;
};

} // namespace ir
} // namespace comp

#endif /* COMPILER_IR_FUNCTION_H_ */
