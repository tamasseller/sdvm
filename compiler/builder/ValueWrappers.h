#ifndef COMPILER_BUILDER_RVALWRAPPER_H_
#define COMPILER_BUILDER_RVALWRAPPER_H_

#include "StatementSink.h"

#include "compiler/ast/Values.h"

#include "assert.h"

namespace comp {

struct RValWrapper
{
	const std::shared_ptr<ast::RValue> val;

	inline RValWrapper(const std::shared_ptr<ast::RValue> &rvalue): val(rvalue) {}

	inline RValWrapper(int integer): val(std::make_shared<ast::Literal>(integer)) {}
	inline RValWrapper(bool logical): val(std::make_shared<ast::Literal>(logical)) {}
	inline RValWrapper(float floating): val(std::make_shared<ast::Literal>(floating)) {}

	inline void operator ()(std::shared_ptr<StatementSink> sink) const {
		sink->add(std::make_shared<ast::ExpressionStatement>(val));
	}

	inline struct LValWrapper operator [](const ast::Field& f) const;

	template<ast::Binary::Operation ifInt, ast::Binary::Operation ifFloat>
	inline RValWrapper selectBinary(const std::shared_ptr<ast::RValue> &o)
	{
		const auto t = val->getType();
		const auto ot = o->getType();
		assert(t.kind == ast::TypeKind::Value);
		assert(ot.kind == ast::TypeKind::Value);

		if(t.primitiveType == ast::PrimitiveType::Integer)
		{
			assert(ot.primitiveType == ast::PrimitiveType::Integer);
			return {std::make_shared<ast::Binary>(ifInt, val, o)};
		}
		else
		{
			assert(t.primitiveType == ast::PrimitiveType::Floating);
			assert(ot.primitiveType == ast::PrimitiveType::Floating);
			return {std::make_shared<ast::Binary>(ifFloat, val, o)};
		}
	}

	inline auto operator +(const RValWrapper &o) {
		return selectBinary<ast::Binary::Operation::AddI, ast::Binary::Operation::AddF>(o.val);
	}

	inline auto operator -(const RValWrapper &o) {
		return selectBinary<ast::Binary::Operation::SubI, ast::Binary::Operation::SubF>(o.val);
	}

	inline auto operator *(const RValWrapper &o) {
		return selectBinary<ast::Binary::Operation::MulI, ast::Binary::Operation::MulF>(o.val);
	}

	inline auto operator /(const RValWrapper &o) {
		return selectBinary<ast::Binary::Operation::DivI, ast::Binary::Operation::DivF>(o.val);
	}

	inline auto operator ==(const RValWrapper &o) {
		return selectBinary<ast::Binary::Operation::Eq, ast::Binary::Operation::Eq>(o.val);
	}

	inline auto operator !=(const RValWrapper &o) {
		return selectBinary<ast::Binary::Operation::Ne, ast::Binary::Operation::Ne>(o.val);
	}

	inline auto operator <(const RValWrapper &o) {
		return selectBinary<ast::Binary::Operation::LtI, ast::Binary::Operation::LtF>(o.val);
	}

	inline auto operator >(const RValWrapper &o) {
		return selectBinary<ast::Binary::Operation::GtI, ast::Binary::Operation::GtF>(o.val);
	}

	inline auto operator <=(const RValWrapper &o) {
		return selectBinary<ast::Binary::Operation::LeI, ast::Binary::Operation::LeF>(o.val);
	}

	inline auto operator >=(const RValWrapper &o) {
		return selectBinary<ast::Binary::Operation::GeI, ast::Binary::Operation::GeF>(o.val);
	}

	template<ast::Binary::Operation ifInt>
	inline RValWrapper checkIntegerBinary(const std::shared_ptr<ast::RValue> &o) {
		const auto t = val->getType();
		const auto ot = o->getType();
		assert(t.kind == ast::TypeKind::Value); // TODO compiler error
		assert(ot.kind == ast::TypeKind::Value); // TODO compiler error
		assert(t.primitiveType == ast::PrimitiveType::Integer); // TODO compiler error
		assert(ot.primitiveType == ast::PrimitiveType::Integer); // TODO compiler error
		return {std::make_shared<ast::Binary>(ifInt, val, o)};
	}

	inline auto operator %(const RValWrapper &o) {
		return checkIntegerBinary<ast::Binary::Operation::Mod>(o.val);
	}

	inline auto operator <<(const RValWrapper &o) {
		return checkIntegerBinary<ast::Binary::Operation::ShlI>(o.val);
	}

	inline auto operator >>(const RValWrapper &o) {
		return checkIntegerBinary<ast::Binary::Operation::ShrU>(o.val);
	}

	inline auto operator &(const RValWrapper &o) {
		return checkIntegerBinary<ast::Binary::Operation::AndI>(o.val);
	}

	inline auto operator |(const RValWrapper &o) {
		return checkIntegerBinary<ast::Binary::Operation::OrI>(o.val);
	}

	inline auto operator ^(const RValWrapper &o) {
		return checkIntegerBinary<ast::Binary::Operation::XorI>(o.val);
	}

	template<ast::Binary::Operation op>
	inline RValWrapper checkLogicBinary(const std::shared_ptr<ast::RValue> &o) {
		const auto t = val->getType();
		const auto ot = o->getType();
		assert(t.kind == ast::TypeKind::Value);  // TODO compiler error
		assert(ot.kind == ast::TypeKind::Value);  // TODO compiler error
		assert(t.primitiveType == ast::PrimitiveType::Logical); // TODO compiler error
		assert(ot.primitiveType == ast::PrimitiveType::Logical); // TODO compiler error
		return {std::make_shared<ast::Binary>(op, val, o)};
	}

	inline auto operator &&(const RValWrapper &o) {
		return checkIntegerBinary<ast::Binary::Operation::And>(o.val);
	}

	inline auto operator ||(const RValWrapper &o) {
		return checkIntegerBinary<ast::Binary::Operation::Or>(o.val);
	}

	inline RValWrapper operator -()
	{
		const auto t = val->getType();
		assert(t.kind == ast::TypeKind::Value);

		if(t.primitiveType == ast::PrimitiveType::Integer)
		{
			return {std::make_shared<ast::Binary>(ast::Binary::Operation::SubI, std::make_shared<ast::Literal>(0), val)};
		}
		else
		{
			assert(t.primitiveType == ast::PrimitiveType::Floating);
			return {std::make_shared<ast::Binary>(ast::Binary::Operation::SubF, std::make_shared<ast::Literal>(0.0f), val)};
		}
	}

	inline RValWrapper operator ~()
	{
		const auto t = val->getType();
		assert(t.kind == ast::TypeKind::Value);
		assert(t.primitiveType == ast::PrimitiveType::Integer);
		return {std::make_shared<ast::Unary>(ast::Unary::Operation::Neg, val)};
	}

	inline RValWrapper operator !()
	{
		const auto t = val->getType();
		assert(t.kind == ast::TypeKind::Value);
		assert(t.primitiveType == ast::PrimitiveType::Logical);
		return {std::make_shared<ast::Unary>(ast::Unary::Operation::Not, val)};
	}

	inline RValWrapper asFloat()
	{
		const auto t = val->getType();
		assert(t.kind == ast::TypeKind::Value);
		if(t.primitiveType == ast::PrimitiveType::Integer)
		{
			return {std::make_shared<ast::Unary>(ast::Unary::Operation::I2F, val)};
		}
		else
		{
			assert(t.primitiveType == ast::PrimitiveType::Floating);
			return val;
		}
	}

	inline RValWrapper asInt()
	{
		const auto t = val->getType();
		assert(t.kind == ast::TypeKind::Value);
		if(t.primitiveType == ast::PrimitiveType::Floating)
		{
			return {std::make_shared<ast::Unary>(ast::Unary::Operation::F2I, val)};
		}
		else
		{
			assert(t.primitiveType == ast::PrimitiveType::Integer);
			return val;
		}
	}
};

struct LValWrapper: RValWrapper
{
	inline LValWrapper(std::shared_ptr<ast::LValue> value): RValWrapper(value) {}

	inline RValWrapper operator =(const RValWrapper &o) const {
		return {std::make_shared<ast::Set>(std::static_pointer_cast<ast::LValue>(val), o.val)};
	}

	inline auto operator =(const LValWrapper &o) const {
		return *this = (const RValWrapper&)o;
	}
};

inline LValWrapper RValWrapper::operator [](const ast::Field& f) const {
	return {std::make_shared<ast::Dereference>(val, f)};
}

} // namespace comp

#endif /* COMPILER_BUILDER_RVALWRAPPER_H_ */
