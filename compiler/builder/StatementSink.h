#ifndef COMPILER_STATEMENTSINK_H_
#define COMPILER_STATEMENTSINK_H_

#include "compiler/ast/Statements.h"

namespace comp {

struct StatementSink
{
	std::shared_ptr<StatementSink> parent;

	virtual std::shared_ptr<ast::Local> addLocal(ast::ValueType type) = 0;
	virtual void add(std::shared_ptr<ast::Statement> stmt) = 0;

	StatementSink(decltype(parent) parent): parent(parent) {}
	inline virtual ~StatementSink() = default;
};

struct BlockSink: StatementSink
{
	const std::shared_ptr<ast::Block> block;

	inline virtual std::shared_ptr<ast::Local> addLocal(ast::ValueType type) override {
		return std::make_shared<ast::Local>(type);
	}

	inline virtual void add(std::shared_ptr<ast::Statement> stmt) override {
		block->stmts.push_back(std::move(stmt));
	}

	inline BlockSink(std::shared_ptr<ast::Block> block, std::shared_ptr<StatementSink> parent = {}):
		StatementSink(parent), block(block) {}
};

struct ConditionalSink: BlockSink
{
	std::shared_ptr<ast::Conditional> conditional;

	inline ConditionalSink(std::shared_ptr<ast::Conditional> conditional, std::shared_ptr<StatementSink> parent = {}):
		BlockSink(conditional->then, parent), conditional(conditional) {}
};

struct LoopSink: BlockSink
{
	std::shared_ptr<ast::Loop> loop;

	inline LoopSink(std::shared_ptr<ast::Loop> loop, std::shared_ptr<StatementSink> parent = {}):
		BlockSink(loop->body, parent), loop(loop) {}
};

}  // namespace comp

#endif /* COMPILER_STATEMENTSINK_H_ */
