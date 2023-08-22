#ifndef COMPILER_MODEL_UNARY_H_
#define COMPILER_MODEL_UNARY_H_

#include "RValue.h"
#include "ValueType.h"

namespace comp {

struct Unary: ValueBase<Unary>
{
	enum class Operation
	{
		Neg, I2F, F2I, Not
	};

	const Operation op;
	const std::shared_ptr<RValue> arg;

	Unary(Operation op, std::shared_ptr<RValue> arg): op(op), arg(arg) {}

	inline virtual ValueType getType() override
	{
		switch(op)
		{
			case Operation::Neg:
			case Operation::F2I:
				return ValueType::integer();
			case Operation::I2F:
				return ValueType::floating();
			case Operation::Not:
				return ValueType::logical();
		}

		return ValueType::native();
	}
};

}  // namespace comp

#endif /* COMPILER_MODEL_UNARY_H_ */
