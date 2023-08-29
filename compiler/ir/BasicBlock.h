#ifndef COMPILER_TAC_CFG_BASICBLOCK_H_
#define COMPILER_TAC_CFG_BASICBLOCK_H_

#include "meta/Operation.h"
#include "meta/Termination.h"
#include "Temporary.h"

#include <memory>
#include <vector>
#include <utility>
#include <map>

namespace comp {
namespace ir {

struct BasicBlock
{
	std::vector<std::shared_ptr<Operation>> code;
	std::shared_ptr<Termination> termination;

	struct DumpContext
	{
		std::map<std::shared_ptr<Temporary>, size_t> ts;

		inline std::string nameOf(const std::shared_ptr<Temporary> &t)
		{
			auto it = ts.find(t);
			return "t" + std::to_string((it != ts.end()) ? it->second : ts.insert({t, ts.size()}).first->second);
		}
	};

	std::string dump(ast::ProgramObjectSet& gi, DumpContext& dc) const;
};

} // namespace ir
} // namespace comp

#endif /* COMPILER_TAC_CFG_BASICBLOCK_H_ */

