#ifndef COMPILER_MODEL_DEREFERENCE_H_
#define COMPILER_MODEL_DEREFERENCE_H_

#include "LValue.h"
#include "Field.h"

namespace comp {

struct Dereference: LValueBase<Dereference>
{
	const std::shared_ptr<RValue> object;
	const Field field;

	inline Dereference(std::shared_ptr<RValue> object, const Field& field): object(object), field(field) {}
	inline virtual ~Dereference() = default;

	inline virtual ValueType getType() override { return field.getType(); }
};

}  // namespace comp

#endif /* COMPILER_MODEL_DEREFERENCE_H_ */
