#ifndef COMPILER_RVALUE_H_
#define COMPILER_RVALUE_H_

#include "ValueType.h"

#include "ValueVisitor.h"

namespace comp {

struct RValue
{
	virtual ValueType getType() = 0;

	virtual void accept(const ValueVisitor& v) const = 0;

	template<class C>
	inline void accept(C&& c) const {
		const LambdaValueVisitor<C> v(std::forward<C>(c));
		this->accept((const ValueVisitor&)v);
	}

	inline virtual ~RValue() = default;
};

template<class Child, class Base = RValue>
struct ValueBase: Base
{
	inline virtual void accept(const ValueVisitor& v) const override final {
		v.visit(*static_cast<const Child*>(this));
	}

	inline virtual ~ValueBase() = default;
};

}  // namespace comp

#endif /* COMPILER_RVALUE_H_ */
