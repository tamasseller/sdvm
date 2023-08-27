#ifndef COMPILER_IR_FUNCTION_H_
#define COMPILER_IR_FUNCTION_H_

#include "BasicBlock.h"

namespace comp {
namespace ir {

struct Function
{
	std::vector<std::shared_ptr<BasicBlock>> blocks;
	inline Function(decltype(blocks) blocks): blocks(blocks) {}

	std::string dump(ast::ProgramObjectSet& gi) const;

	static std::shared_ptr<Function> from(std::shared_ptr<ast::Function>);
};

} // namespace ir
} // namespace comp

#endif /* COMPILER_IR_FUNCTION_H_ */
