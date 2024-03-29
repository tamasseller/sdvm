#ifndef COMPILER_FUNCTIONBUILDER_H_
#define COMPILER_FUNCTIONBUILDER_H_

#include "Helpers.h"

#include "compiler/ast/Class.h"
#include "compiler/ast/Function.h"
#include "compiler/ast/Statements.h"
#include "compiler/ast/Values.h"

#include "compiler/internal/Compiler.h"

#include "program/Program.h"

#include <optional>

namespace comp {

class Line;

class FunctionBuilder
{
	friend class ProgramBuilder;

	std::shared_ptr<ast::Function> data;
	std::shared_ptr<StatementSink> currentBlock;

	inline FunctionBuilder(std::shared_ptr<ast::Function> data): data(data), currentBlock(std::make_shared<BlockSink>(data->body)) {}

public:
	static inline FunctionBuilder make(std::vector<ast::ValueType> ret, std::vector<ast::ValueType> args) {
		return std::make_shared<ast::Function>(ret, args);
	}

	inline LValWrapper operator [](size_t n) const
	{
		assert(n < data->args.size());
		return {std::make_shared<ast::Argument>(data->args[n], n)};
	}

	template<class C>
	inline auto operator<<=(C&& stmt) {
		return stmt(currentBlock);
	}

	inline auto operator<<=(const RValWrapper &rval) {
		return rval(currentBlock);
	}

	template<class... Args>
	inline RValWrapper operator()(Args&&... args)
	{
		std::shared_ptr<const ast::RValue> as[] = {args.val...};
		std::vector<std::shared_ptr<const ast::RValue>> a{as, as + sizeof(as)/sizeof(as[0])};

		assert(a.size() == data->args.size()); // compile error, TODO check types

		return {std::make_shared<ast::Call>(data, a)};
	}

	Compiler build() const {
		return {data};
	}
};

} //namespace comp

#endif /* COMPILER_FUNCTIONBUILDER_H_ */
