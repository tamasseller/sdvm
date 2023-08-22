#ifndef COMPILER_MODEL_ARGUMENT_H_
#define COMPILER_MODEL_ARGUMENT_H_

#include "LValue.h"

namespace comp {

struct Argument: LValueBase<Argument>
{
	const ValueType type;
	const size_t idx;

	inline Argument(ValueType type, size_t idx): type(type), idx(idx) {}

	inline virtual ValueType getType() override { return type; }
};

}  // namespace comp

#endif /* COMPILER_MODEL_ARGUMENT_H_ */
