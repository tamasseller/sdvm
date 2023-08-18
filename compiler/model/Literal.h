#ifndef COMPILER_MODEL_LITERAL_H_
#define COMPILER_MODEL_LITERAL_H_

#include "RValue.h"

namespace comp {

struct Literal: RValue
{
	const ValueType type;

	union
	{
		const int integer;
		const float floating;
		const bool logical;
	};

	inline Literal(int integer): type(ValueType::integer()), integer(integer) {}
	inline Literal(float floating): type(ValueType::floating()), floating(floating) {}
	inline Literal(bool logical): type(ValueType::logical()), logical(logical) {}

	inline virtual ValueType getType() override { return type; }
};

}  // namespace comp

#endif /* COMPILER_MODEL_LITERAL_H_ */
