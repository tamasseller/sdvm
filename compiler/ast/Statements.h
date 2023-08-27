#ifndef COMPILER_MODEL_STATEMENTTYPES_H_
#define COMPILER_MODEL_STATEMENTTYPES_H_

#include "meta/Value.h"
#include "meta/Statement.h"

namespace comp {
namespace ast {

struct ExpressionStatement: StatementBase<ExpressionStatement>
{
	std::shared_ptr<const RValue> val;
	inline ExpressionStatement(decltype(val) val): val(val) {}
};

struct Block: StatementBase<Block>
{
	std::vector<std::shared_ptr<Statement>> stmts;
};

struct Declaration: StatementBase<Declaration>
{
	const std::shared_ptr<const Local> local;
	const std::shared_ptr<const RValue> initializer;

	inline Declaration(std::shared_ptr<const Local> local, std::shared_ptr<const RValue> initializer): local(local), initializer(initializer) {}
	inline virtual ~Declaration() = default;
};

struct Conditional: StatementBase<Conditional>
{
	const std::shared_ptr<const RValue> condition;
	const std::shared_ptr<Block> then = std::make_shared<Block>();
	const std::shared_ptr<Block> otherwise = std::make_shared<Block>();

	inline Conditional(std::shared_ptr<const RValue> condition): condition(condition) {}
};

struct Loop: StatementBase<Loop>
{
	const std::shared_ptr<Block> body = std::make_shared<Block>();
};

struct Break: StatementBase<Break>
{
	const std::shared_ptr<Loop> loop;

	inline Break(std::shared_ptr<Loop> loop): loop(loop) {}
};

struct Continue: StatementBase<Continue>
{
	const std::shared_ptr<Loop> loop;

	inline Continue(std::shared_ptr<Loop> loop): loop(loop) {}
};

struct Return: StatementBase<Return>
{
	std::vector<std::shared_ptr<const RValue>> value;

	inline Return(const std::vector<std::shared_ptr<const RValue>> &value = {}): value(value) {}
	inline virtual ~Return() = default;
};

} // namespace ast
} // namespace comp

#endif /* COMPILER_MODEL_STATEMENTTYPES_H_ */
