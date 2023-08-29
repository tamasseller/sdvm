#ifndef COMPILER_IR_META_TERMINATION_H_
#define COMPILER_IR_META_TERMINATION_H_

#include <utility>

namespace comp {
namespace ir {

#define _TERMINATION_TYPES() \
		X(Always) \
		X(Conditional) \
		X(Leave) \

#define X(n) class n;
_TERMINATION_TYPES()
#undef X

struct TerminationVisitor
{
#define X(n) virtual void visit(const n&) const = 0;
_TERMINATION_TYPES()
#undef X

	inline virtual ~TerminationVisitor() = default;
};

template<class C>
struct LambdaTerminationVisitor: TerminationVisitor
{
	C c;

	LambdaTerminationVisitor(C&& c): c(std::forward<C>(c)) {}
	inline virtual ~LambdaTerminationVisitor() = default;

#define X(n) virtual void visit(const n &v) const override final { c(v); };
_TERMINATION_TYPES()
#undef X
};

struct Termination
{
	virtual void accept(const TerminationVisitor& v) const = 0;

	template<class C>
	inline void accept(C&& c) const {
		const LambdaTerminationVisitor<C> v(std::forward<C>(c));
		this->accept((const TerminationVisitor&)v);
	}

	inline virtual ~Termination() = default;
};

template<class Child>
struct TerminationBase: Termination
{
	inline virtual void accept(const TerminationVisitor& v) const override final {
		v.visit(*static_cast<const Child*>(this));
	}

	inline virtual ~TerminationBase() = default;
};

} // namespace ir
} // namespace comp

#endif /* COMPILER_IR_META_TERMINATION_H_ */
