#ifndef COMPILER_MODEL_BINARY_H_
#define COMPILER_MODEL_BINARY_H_

#include "RValue.h"
#include "ValueType.h"

namespace comp {

struct Binary: ValueBase<Binary>
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
		DivF,
		Eq,
		Ne,
		LtI,
		GtI,
		LeI,
		GeI,
		LtU,
		GtU,
		LeU,
		GeU,
		LtF,
		GtF,
		LeF,
		GeF,
		And,
		Or
	};

	const Operation op;
	const std::shared_ptr<RValue> first, second;

	Binary(Operation op, std::shared_ptr<RValue> first, std::shared_ptr<RValue> second): op(op), first(first), second(second) {}

	inline virtual ValueType getType() override
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
			case Operation::Eq:
			case Operation::Ne:
			case Operation::LtI:
			case Operation::GtI:
			case Operation::LeI:
			case Operation::GeI:
			case Operation::LtU:
			case Operation::GtU:
			case Operation::LeU:
			case Operation::GeU:
			case Operation::LtF:
			case Operation::GtF:
			case Operation::LeF:
			case Operation::GeF:
			case Operation::And:
			case Operation::Or:
				return ValueType::logical();
		}

		return ValueType::native();
	}
};

}  // namespace comp

#endif /* COMPILER_MODEL_BINARY_H_ */
