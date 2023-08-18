#ifndef COMPILER_MODEL_BINARY_H_
#define COMPILER_MODEL_BINARY_H_

#include "RValue.h"

namespace comp {

struct Binary: RValue
{
	enum class Operation
	{
		AddI,
		MulI,
		SubI,
		DivI,
		Mod,
		ShlI,
		ShrI,
		ShrU,
		AndI,
		OrI,
		XorI,
		AddF,
		MulF,
		SubF,
		DivF
	};

	const Operation op;
	const std::shared_ptr<RValue> first, second;

	Binary(Operation op, std::shared_ptr<RValue> first, std::shared_ptr<RValue> second): op(op), first(first), second(second) {}

	virtual ValueType getType()
	{
		switch(op)
		{
			case Operation::AddI:
			case Operation::MulI:
			case Operation::SubI:
			case Operation::DivI:
			case Operation::Mod:
			case Operation::ShlI:
			case Operation::ShrI:
			case Operation::ShrU:
			case Operation::AndI:
			case Operation::OrI:
			case Operation::XorI:
				return ValueType::integer();
			case Operation::AddF:
			case Operation::MulF:
			case Operation::SubF:
			case Operation::DivF:
				return ValueType::floating();
		}

		return ValueType::native();
	}
};

}  // namespace comp

#endif /* COMPILER_MODEL_BINARY_H_ */
