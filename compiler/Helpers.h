#ifndef COMPILER_HELPERS_H_
#define COMPILER_HELPERS_H_

#include "StatementSink.h"

#include "model/Field.h"
#include "model/Unary.h"
#include "model/Binary.h"
#include "model/Create.h"
#include "model/Ternary.h"
#include "model/Literal.h"
#include "model/Dereference.h"
#include "model/ExpressionStatement.h"

#include "assert.h"

namespace comp {

struct RValWrapper
{
	const std::shared_ptr<RValue> val;

	inline RValWrapper(const std::shared_ptr<RValue> &rvalue): val(rvalue) {}

	inline RValWrapper(int integer): val(std::make_shared<Literal>(integer)) {}
	inline RValWrapper(bool logical): val(std::make_shared<Literal>(logical)) {}
	inline RValWrapper(float floating): val(std::make_shared<Literal>(floating)) {}

	inline void operator ()(std::shared_ptr<StatementSink> sink) const {
		sink->exprStmt(val);
	}

	inline struct LValWrapper operator [](const Field& f) const;

	template<Binary::Operation ifInt, Binary::Operation ifFloat>
	inline RValWrapper selectBinary(const std::shared_ptr<RValue> &o)
	{
		const auto t = val->getType();
		const auto ot = o->getType();
		assert(t.kind == TypeKind::Value);
		assert(ot.kind == TypeKind::Value);

		if(t.primitiveType == PrimitiveType::Integer)
		{
			assert(ot.primitiveType == PrimitiveType::Integer);
			return {std::make_shared<Binary>(ifInt, val, o)};
		}
		else
		{
			assert(t.primitiveType == PrimitiveType::Floating);
			assert(ot.primitiveType == PrimitiveType::Floating);
			return {std::make_shared<Binary>(ifFloat, val, o)};
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

	inline auto operator ==(const RValWrapper &o) {
		return selectBinary<Binary::Operation::Eq, Binary::Operation::Eq>(o.val);
	}

	inline auto operator !=(const RValWrapper &o) {
		return selectBinary<Binary::Operation::Ne, Binary::Operation::Ne>(o.val);
	}

	inline auto operator <(const RValWrapper &o) {
		return selectBinary<Binary::Operation::LtI, Binary::Operation::LtF>(o.val);
	}

	inline auto operator >(const RValWrapper &o) {
		return selectBinary<Binary::Operation::GtI, Binary::Operation::GtF>(o.val);
	}

	inline auto operator <=(const RValWrapper &o) {
		return selectBinary<Binary::Operation::LeI, Binary::Operation::LeF>(o.val);
	}

	inline auto operator >=(const RValWrapper &o) {
		return selectBinary<Binary::Operation::GeI, Binary::Operation::GeF>(o.val);
	}

	template<Binary::Operation ifInt>
	inline RValWrapper checkIntegerBinary(const std::shared_ptr<RValue> &o) {
		const auto t = val->getType();
		const auto ot = o->getType();
		assert(t.kind == TypeKind::Value);
		assert(ot.kind == TypeKind::Value);
		assert(t.primitiveType == PrimitiveType::Integer);
		assert(ot.primitiveType == PrimitiveType::Integer);
		return {std::make_shared<Binary>(ifInt, val, o)};
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

	template<Binary::Operation op>
	inline RValWrapper checkLogicBinary(const std::shared_ptr<RValue> &o) {
		const auto t = val->getType();
		const auto ot = o->getType();
		assert(t.kind == TypeKind::Value);
		assert(ot.kind == TypeKind::Value);
		assert(t.primitiveType == PrimitiveType::Logical);
		assert(ot.primitiveType == PrimitiveType::Logical);
		return {std::make_shared<Binary>(op, val, o)};
	}

	inline auto operator &&(const RValWrapper &o) {
		return checkIntegerBinary<Binary::Operation::And>(o.val);
	}

	inline auto operator ||(const RValWrapper &o) {
		return checkIntegerBinary<Binary::Operation::Or>(o.val);
	}

	inline RValWrapper operator -()
	{
		const auto t = val->getType();
		assert(t.kind == TypeKind::Value);

		if(t.primitiveType == PrimitiveType::Integer)
		{
			return {std::make_shared<Binary>(Binary::Operation::SubI, std::make_shared<Literal>(0), val)};
		}
		else
		{
			assert(t.primitiveType == PrimitiveType::Floating);
			return {std::make_shared<Binary>(Binary::Operation::SubF, std::make_shared<Literal>(0.0f), val)};
		}
	}

	inline RValWrapper operator ~()
	{
		const auto t = val->getType();
		assert(t.kind == TypeKind::Value);
		assert(t.primitiveType == PrimitiveType::Integer);
		return {std::make_shared<Unary>(Unary::Operation::Neg, val)};
	}

	inline RValWrapper operator !()
	{
		const auto t = val->getType();
		assert(t.kind == TypeKind::Value);
		assert(t.primitiveType == PrimitiveType::Integer);
		return {std::make_shared<Unary>(Unary::Operation::Not, val)};
	}

	inline RValWrapper asFloat()
	{
		const auto t = val->getType();
		assert(t.kind == TypeKind::Value);
		if(t.primitiveType == PrimitiveType::Integer)
		{
			return {std::make_shared<Unary>(Unary::Operation::I2F, val)};
		}
		else
		{
			assert(t.primitiveType == PrimitiveType::Floating);
			return val;
		}
	}

	inline RValWrapper asInt()
	{
		const auto t = val->getType();
		assert(t.kind == TypeKind::Value);
		if(t.primitiveType == PrimitiveType::Floating)
		{
			return {std::make_shared<Unary>(Unary::Operation::F2I, val)};
		}
		else
		{
			assert(t.primitiveType == PrimitiveType::Integer);
			return val;
		}
	}
};

struct LValWrapper: RValWrapper
{
	inline LValWrapper(std::shared_ptr<LValue> value): RValWrapper(value) {}

	inline auto operator =(const RValWrapper &o) const
	{
		return [target{std::static_pointer_cast<LValue>(val)}, value{o.val}](std::shared_ptr<StatementSink>& sink)
		{
			sink->set(target, value);
		};
	}

	inline auto operator =(const LValWrapper &o) const
	{
		return *this = (const RValWrapper&)o;
	}
};

inline LValWrapper RValWrapper::operator [](const Field& f) const {
	return {std::make_shared<Dereference>(val, f)};
}

inline auto declaration(ValueType type, const RValWrapper& initializer)
{
	return [type, initializer](std::shared_ptr<StatementSink>& sink) -> LValWrapper
	{
		return {sink->addLocal(type, initializer.val)};
	};
}

inline auto declaration(const RValWrapper& initializer) {
	return declaration(initializer.val->getType(), initializer);
}

inline auto ret()
{
	return [](std::shared_ptr<StatementSink>& sink)
	{
		return sink->ret();
	};
}

inline auto ret(const RValWrapper& val)
{
	return [val{val.val}](std::shared_ptr<StatementSink>& sink)
	{
		std::vector<std::shared_ptr<RValue>> a;
		a.push_back(val);
		return sink->ret(a);
	};
}

inline RValWrapper null(std::make_shared<Create>(nullptr));

inline RValWrapper ternary(const RValWrapper& condition, const RValWrapper& then, const RValWrapper& otherwise) {
	return {std::make_shared<Ternary>(condition.val, then.val, otherwise.val)};
}

inline auto conditional(const RValWrapper& condition)
{
	return [condition{condition.val}](std::shared_ptr<StatementSink>& sink)
	{
		// TODO cond = new Conditional(condition)
		// TODO sink->add(cond)
		// TODO sink = new ForkSink(cond, sink)
	};
}

inline auto otherwise()
{
	return [](std::shared_ptr<StatementSink>& sink)
	{
		// TODO forkSink = std::dynamic_cast<ForkSink>(sink)
		// TODO sink->block = forkSink->cond.otherwise;
	};
}

inline auto endif()
{
	return [](std::shared_ptr<StatementSink>& sink)
	{
		// TODO forkSink = std::dynamic_cast<ForkSink>(sink)
		// TODO sink = forkSink->parent;
	};
}


} // namespace comp

#endif /* COMPILER_HELPERS_H_ */
