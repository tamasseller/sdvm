#ifndef COMPILER_STATEMENTSINK_H_
#define COMPILER_STATEMENTSINK_H_

#include "model/Set.h"
#include "model/Block.h"
#include "model/Local.h"
#include "model/Declaration.h"

namespace comp {

struct StatementSink
{
	const std::shared_ptr<Block> block;

	inline std::shared_ptr<Local> addLocal(ValueType type)
	{
		auto ret = std::make_shared<Local>(type);
		block->stmts.push_back(std::make_shared<Declaration>(ret));
		return ret;
	}

	inline void set(std::shared_ptr<LValue> target, std::shared_ptr<RValue> value)
	{
		block->stmts.push_back(std::make_shared<Set>(target, value));
	}

	inline StatementSink(std::shared_ptr<Block> block): block(block) {}
};

}  // namespace comp

#endif /* COMPILER_STATEMENTSINK_H_ */
