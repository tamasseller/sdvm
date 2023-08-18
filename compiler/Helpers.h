#ifndef COMPILER_HELPERS_H_
#define COMPILER_HELPERS_H_

#include "StatementSink.h"

#include "model/ValueType.h"
#include "model/Binary.h"
#include "model/Literal.h"

namespace comp {

struct RValWrapper
{
	const std::shared_ptr<RValue> val;

	inline RValWrapper(std::shared_ptr<RValue> rvalue): val(rvalue) {}

	inline RValWrapper(int integer): val(std::make_shared<Literal>(integer)) {}
	inline RValWrapper(bool logical): val(std::make_shared<Literal>(logical)) {}
	inline RValWrapper(float floating): val(std::make_shared<Literal>(floating)) {}

	template<Binary::Operation ifInt, Binary::Operation ifFloat>
	inline std::shared_ptr<RValue> selectBinary(const std::shared_ptr<RValue> &o)
	{
		const auto t = val->getType();
		const auto ot = o->getType();
		assert(t.kind == TypeKind::Value);
		assert(ot.kind == TypeKind::Value);

		if(t.primitiveType == PrimitiveType::Integer)
		{
			assert(ot.primitiveType == PrimitiveType::Integer);
			return std::make_shared<Binary>(ifInt, val, o);
		}
		else
		{
			assert(ot.primitiveType == PrimitiveType::Floating);
			return std::make_shared<Binary>(ifFloat, val, o);
		}
	}

	inline auto operator +(const RValWrapper &o) {
		return selectBinary<Binary::Operation::AddI, Binary::Operation::AddF>(o.val);
	}

	inline auto operator -(const RValWrapper &o) {
		return selectBinary<Binary::Operation::SubI, Binary::Operation::SubF>(o.val);
	}

	inline auto operator *(const RValWrapper &o) {
		return selectBinary<Binary::Operation::MulI, Binary::Operation::MulF>(o.val);
	}

	inline auto operator /(const RValWrapper &o) {
		return selectBinary<Binary::Operation::DivI, Binary::Operation::DivF>(o.val);
	}

	template<Binary::Operation ifInt>
	inline std::shared_ptr<RValue> checkIntegerBinary(const std::shared_ptr<RValue> &o) {
		const auto t = val->getType();
		const auto ot = o->getType();
		assert(t.kind == TypeKind::Value);
		assert(ot.kind == TypeKind::Value);
		assert(t.primitiveType == PrimitiveType::Integer);
		assert(ot.primitiveType == PrimitiveType::Integer);
		return std::make_shared<Binary>(ifInt, val, o);
	}

	inline auto operator %(const RValWrapper &o) {
		return checkIntegerBinary<Binary::Operation::Mod>(o.val);
	}

	inline auto operator <<(const RValWrapper &o) {
		return checkIntegerBinary<Binary::Operation::ShlI>(o.val);
	}

	inline auto operator >>(const RValWrapper &o) {
		return checkIntegerBinary<Binary::Operation::ShrU>(o.val);
	}

	inline auto operator &(const RValWrapper &o) {
		return checkIntegerBinary<Binary::Operation::AndI>(o.val);
	}

	inline auto operator |(const RValWrapper &o) {
		return checkIntegerBinary<Binary::Operation::OrI>(o.val);
	}

	inline auto operator ^(const RValWrapper &o) {
		return checkIntegerBinary<Binary::Operation::XorI>(o.val);
	}
};

struct LValWrapper: RValWrapper
{
	inline LValWrapper(std::shared_ptr<LValue> value): RValWrapper(value) {}

	inline auto operator =(const RValWrapper &o)
	{
		return [target{std::static_pointer_cast<LValue>(this->val)}, value{o.val}](std::shared_ptr<StatementSink>& sink)
		{
			sink->set(target, value);
		};
	}
};

inline auto declaration(ValueType type)
{
	return [type](std::shared_ptr<StatementSink>& sink) -> LValWrapper
	{
		return {sink->addLocal(type)};
	};
}

} // namespace comp

#endif /* COMPILER_HELPERS_H_ */
