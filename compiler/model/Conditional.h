#ifndef COMPILER_MODEL_CONDITIONAL_H_
#define COMPILER_MODEL_CONDITIONAL_H_

#include "RValue.h"
#include "Statement.h"

namespace comp {

struct Conditional: StatementBase<Conditional>
{
	const std::shared_ptr<RValue> condition;
	const std::shared_ptr<Block> then = std::make_shared<Block>();
	const std::shared_ptr<Block> otherwise = std::make_shared<Block>();

	inline Conditional(std::shared_ptr<RValue> condition): condition(condition) {}
};

}  // namespace comp

#endif /* COMPILER_MODEL_CONDITIONAL_H_ */
