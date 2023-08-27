#ifndef COMPILER_TAC_CFG_BASICBLOCK_H_
#define COMPILER_TAC_CFG_BASICBLOCK_H_

#include "compiler/ast/Values.h"
#include "compiler/ast/Statements.h"

#include <memory>

namespace comp {
namespace ir {

struct BasicBlock
{
	std::shared_ptr<ast::Block> code = std::make_shared<ast::Block>();
	std::shared_ptr<const ast::RValue> decisionInput;
	std::shared_ptr<BasicBlock> then, otherwise;

	std::string dump(ast::ProgramObjectSet& gi) const;
};

} // namespace ir
} // namespace comp

#endif /* COMPILER_TAC_CFG_BASICBLOCK_H_ */

