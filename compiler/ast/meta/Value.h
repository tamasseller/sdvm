#ifndef COMPILER_MODEL_EXPRESSION_H_
#define COMPILER_MODEL_EXPRESSION_H_

#include "compiler/ast/ValueType.h"

#include "assert.h"

namespace comp {
namespace ast {

#define _VALUE_TYPES() \
	X(Set) \
	X(Call) \
	X(Local) \
	X(Unary) \
	X(Binary) \
	X(Create) \
	X(Global) \
	X(Literal) \
	X(Ternary) \
	X(Argument) \
	X(Dereference)

#define X(n) class n;
_VALUE_TYPES()
#undef X

struct ValueVisitor
{
#define X(n) virtual void visit(const n&) const = 0;
	_VALUE_TYPES()
#undef X

	inline virtual ~ValueVisitor() = default;
};

template<class C>
struct LambdaValueVisitor: ValueVisitor
{
	C c;

	LambdaValueVisitor(C&& c): c(std::forward<C>(c)) {}
	inline virtual ~LambdaValueVisitor() = default;

#define X(n) virtual void visit(const n &v) const override final { c(v); };
	_VALUE_TYPES()
#undef X
};

struct RValue
{
	virtual ValueType getType() const = 0;

	virtual void accept(const ValueVisitor& v) const = 0;

	template<class C>
	inline void accept(C&& c) const {
		const LambdaValueVisitor<C> v(std::forward<C>(c));
		this->accept((const ValueVisitor&)v);
	}

	inline virtual ~RValue() = default;
};

template<class Child, class Base = RValue>
struct ValueBase: Base, std::enable_shared_from_this<Child>
{
	inline virtual void accept(const ValueVisitor& v) const override final {
		v.visit(*static_cast<const Child*>(this));
	}

	inline virtual ~ValueBase() = default;
};

struct LValue: RValue {};

template<class Child>
struct LValueBase: ValueBase<Child, LValue> {};

} // namespace ast
} // namespace comp

#endif /* COMPILER_MODEL_EXPRESSION_H_ */
