#ifndef COMPILER_TAC_CFG_BASICBLOCK_H_
#define COMPILER_TAC_CFG_BASICBLOCK_H_

#include "meta/Operation.h"
#include "Temporary.h"

#include <memory>
#include <vector>

namespace comp {
namespace ir {

struct BasicBlock
{
	std::vector<std::shared_ptr<Operation>> code;
	std::shared_ptr<Temporary> decisionInput;
	std::shared_ptr<BasicBlock> then, otherwise;

	std::string dump(ast::ProgramObjectSet& gi) const;
};

} // namespace ir
} // namespace comp

#endif /* COMPILER_TAC_CFG_BASICBLOCK_H_ */

