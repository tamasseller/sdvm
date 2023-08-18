#ifndef COMPILER_MODEL_SET_H_
#define COMPILER_MODEL_SET_H_

#include "Statement.h"
#include "LValue.h"

namespace comp {

struct Set: Statement
{
	std::shared_ptr<LValue> target;
	std::shared_ptr<RValue> value;

	inline Set(std::shared_ptr<LValue> target, std::shared_ptr<RValue> value): target(target), value(value) {}
};

}  // namespace comp

#endif /* COMPILER_MODEL_SET_H_ */
