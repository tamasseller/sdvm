#ifndef COMPILER_STATEMENTSINK_H_
#define COMPILER_STATEMENTSINK_H_

#include "compiler/model/Loop.h"
#include "compiler/model/Block.h"
#include "compiler/model/Conditional.h"

namespace comp {

struct StatementSink
{
	std::shared_ptr<StatementSink> parent;

	virtual std::shared_ptr<Local> addLocal(ValueType type) = 0;
	virtual void add(std::shared_ptr<Statement> stmt) = 0;

	StatementSink(decltype(parent) parent): parent(parent) {}
	inline virtual ~StatementSink() = default;
};

struct BlockSink: StatementSink
{
	const std::shared_ptr<Block> block;

	inline virtual std::shared_ptr<Local> addLocal(ValueType type) override {
		return std::make_shared<Local>(type);
	}

	inline virtual void add(std::shared_ptr<Statement> stmt) override {
		block->stmts.push_back(std::move(stmt));
	}

	inline BlockSink(std::shared_ptr<Block> block, std::shared_ptr<StatementSink> parent = {}):
		StatementSink(parent), block(block) {}
};

struct ConditionalSink: BlockSink
{
	std::shared_ptr<Conditional> conditional;

	inline ConditionalSink(std::shared_ptr<Conditional> conditional, std::shared_ptr<StatementSink> parent = {}):
		BlockSink(conditional->then, parent), conditional(conditional) {}
};

struct LoopSink: BlockSink
{
	std::shared_ptr<Loop> loop;

	inline LoopSink(std::shared_ptr<Loop> loop, std::shared_ptr<StatementSink> parent = {}):
		BlockSink(loop->body, parent), loop(loop) {}
};

}  // namespace comp

#endif /* COMPILER_STATEMENTSINK_H_ */
