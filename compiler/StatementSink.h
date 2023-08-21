#ifndef COMPILER_STATEMENTSINK_H_
#define COMPILER_STATEMENTSINK_H_

#include "model/Set.h"
#include "model/Call.h"
#include "model/Block.h"
#include "model/Local.h"
#include "model/Return.h"
#include "model/Function.h"
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

	inline void ret()
	{
		block->stmts.push_back(std::make_shared<Return>());
	}

	inline void call(std::shared_ptr<Function> fn, std::vector<std::shared_ptr<RValue>> args)
	{
		block->stmts.push_back(std::make_shared<Call>(fn, args));
	}

	inline StatementSink(std::shared_ptr<Block> block): block(block) {}
};

}  // namespace comp

#endif /* COMPILER_STATEMENTSINK_H_ */
