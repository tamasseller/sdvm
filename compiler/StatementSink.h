#ifndef COMPILER_STATEMENTSINK_H_
#define COMPILER_STATEMENTSINK_H_

#include "model/Set.h"
#include "model/Call.h"
#include "model/Block.h"
#include "model/Local.h"
#include "model/Return.h"
#include "model/Function.h"
#include "model/Declaration.h"
#include "model/ExpressionStatement.h"

namespace comp {

struct StatementSink
{
	const std::shared_ptr<Block> block;

	inline std::shared_ptr<Local> addLocal(ValueType type, std::shared_ptr<RValue> initializer)
	{
		auto ret = std::make_shared<Local>(type);
		block->stmts.push_back(std::make_shared<Declaration>(ret, initializer));
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

	inline void ret(std::vector<std::shared_ptr<RValue>> value)
	{
		block->stmts.push_back(std::make_shared<Return>(value));
	}

	inline void exprStmt(std::shared_ptr<RValue> value)
	{
		block->stmts.push_back(std::make_shared<ExpressionStatement>(value));
	}

	inline StatementSink(std::shared_ptr<Block> block): block(block) {}
};

}  // namespace comp

#endif /* COMPILER_STATEMENTSINK_H_ */
