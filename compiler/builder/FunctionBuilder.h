#ifndef COMPILER_FUNCTIONBUILDER_H_
#define COMPILER_FUNCTIONBUILDER_H_

#include "Helpers.h"

#include "compiler/model/Call.h"
#include "compiler/model/Class.h"
#include "compiler/model/Argument.h"
#include "compiler/model/Function.h"
#include "compiler/model/Statement.h"

#include "compiler/Compile.h"

#include "program/Program.h"

#include <optional>

namespace comp {

class Line;

class FunctionBuilder
{
	friend class ProgramBuilder;

	std::shared_ptr<Function> data;
	std::shared_ptr<StatementSink> currentBlock;

	inline FunctionBuilder(std::shared_ptr<Function> data): data(data), currentBlock(std::make_shared<BlockSink>(data->body)) {}

public:
	static inline FunctionBuilder make(std::vector<ValueType> ret, std::vector<ValueType> args) {
		return std::make_shared<Function>(ret, args);
	}

	inline LValWrapper operator [](size_t n) const
	{
		assert(n < data->args.size());
		return {std::make_shared<Argument>(data->args[n], n)};
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
		std::shared_ptr<RValue> as[] = {args.val...};
		std::vector<std::shared_ptr<RValue>> a{as, as + sizeof(as)/sizeof(as[0])};

		assert(a.size() == data->args.size()); // compile error, TODO check types

		return {std::make_shared<Call>(data, a)};
	}

	prog::Program compile() const {
		return comp::compile(data);
	}
};

} //namespace comp

#endif /* COMPILER_FUNCTIONBUILDER_H_ */
