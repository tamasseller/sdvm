#ifndef COMPILER_MODEL_CALL_H_
#define COMPILER_MODEL_CALL_H_

#include "RValue.h"

#include "Function.h"

#include "assert.h"

namespace comp {

struct Call: RValue, StatementBase<Call>
{
	std::shared_ptr<Function> fn;
	std::vector<std::shared_ptr<RValue>> args;

	inline Call(std::shared_ptr<Function> fn, std::vector<std::shared_ptr<RValue>> args): fn(fn), args(args) {}

	inline virtual ValueType getType() override
	{
		assert(fn->ret.size() == 1); // compile error

		return fn->ret[0];
	}
};

}  // namespace comp

#endif /* COMPILER_MODEL_CALL_H_ */
