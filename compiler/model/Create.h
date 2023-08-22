#ifndef COMPILER_MODEL_CREATE_H_
#define COMPILER_MODEL_CREATE_H_

#include "RValue.h"

namespace comp {

struct Create: ValueBase<Create>
{
	const std::shared_ptr<Class> type;

	inline Create(decltype(type) type): type(type) {}

	inline virtual ValueType getType() override { return ValueType::reference(type); }
};

}  // namespace comp

#endif /* COMPILER_MODEL_CREATE_H_ */
