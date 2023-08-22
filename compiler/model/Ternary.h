#ifndef COMPILER_MODEL_TERNARY_H_
#define COMPILER_MODEL_TERNARY_H_

#include "RValue.h"

namespace comp {

struct Ternary: ValueBase<Ternary>
{
	const std::shared_ptr<RValue> condition, then, otherwise;

	Ternary(std::shared_ptr<RValue> condition, std::shared_ptr<RValue> then, std::shared_ptr<RValue> otherwise):
		condition(condition), then(then), otherwise(otherwise) {}

	inline virtual ValueType getType() override {
		// TODO assert(then->getType() == otherwise->getType());
		return then->getType();
	}
};

}  // namespace comp

#endif /* COMPILER_MODEL_TERNARY_H_ */
