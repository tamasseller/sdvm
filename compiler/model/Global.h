#ifndef COMPILER_MODEL_GLOBAL_H_
#define COMPILER_MODEL_GLOBAL_H_

#include "LValue.h"
#include "Field.h"

namespace comp {

struct Global: LValueBase<Global>
{
	const StaticField field;

	inline Global(const StaticField& field): field(field) {}
	inline virtual ~Global() = default;

	inline virtual ValueType getType() override { return field.getType(); }
};

}  // namespace comp


#endif /* COMPILER_MODEL_GLOBAL_H_ */
