#ifndef COMPILER_MODEL_LOCAL_H_
#define COMPILER_MODEL_LOCAL_H_

#include "LValue.h"

namespace comp {

struct Local: LValueBase<Local>
{
	const ValueType type;

	inline Local(ValueType type): type(type) {}

	inline virtual ValueType getType() override { return type; }
};

}  // namespace comp

#endif /* COMPILER_MODEL_LOCAL_H_ */
