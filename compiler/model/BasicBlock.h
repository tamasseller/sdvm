#ifndef COMPILER_TAC_CFG_BASICBLOCK_H_
#define COMPILER_TAC_CFG_BASICBLOCK_H_

#include "compiler/model/ExpressionNodes.h"
#include "compiler/model/StatementTypes.h"

#include <memory>

namespace comp {

struct BasicBlock
{
	std::shared_ptr<Block> code = std::make_shared<Block>();
	std::shared_ptr<const RValue> decisionInput;
	std::shared_ptr<BasicBlock> then, otherwise;
};

}  // namespace comp

#endif /* COMPILER_TAC_CFG_BASICBLOCK_H_ */

