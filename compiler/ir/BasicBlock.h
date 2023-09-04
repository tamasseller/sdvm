#ifndef COMPILER_TAC_CFG_BASICBLOCK_H_
#define COMPILER_TAC_CFG_BASICBLOCK_H_

#include "compiler/common/meta/Annotation.h"

#include "meta/Operation.h"
#include "meta/Termination.h"

#include "Temporary.h"

#include <memory>
#include <vector>
#include <utility>
#include <map>

#include "assert.h"

namespace comp {
namespace ir {

struct BasicBlock
{
	std::vector<std::shared_ptr<Operation>> code;
	std::shared_ptr<Termination> termination;
	std::vector<std::shared_ptr<Annotation>> annotations;

	struct DumpContext
	{
		std::map<std::shared_ptr<Variable>, size_t> ts;
		std::string nameOf(const std::shared_ptr<Temporary> &t);
	};

	std::string dump(ast::ProgramObjectSet& gi, DumpContext& dc) const;
};

} // namespace ir
} // namespace comp

#endif /* COMPILER_TAC_CFG_BASICBLOCK_H_ */

