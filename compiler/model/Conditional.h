#ifndef COMPILER_MODEL_CONDITIONAL_H_
#define COMPILER_MODEL_CONDITIONAL_H_

#include "RValue.h"
#include "Statement.h"

#include <memory>
#include <vector>

namespace comp {

struct Conditional
{
	std::shared_ptr<RValue> condition;
	Block then, otherwise;
};

}  // namespace comp

#endif /* COMPILER_MODEL_CONDITIONAL_H_ */
