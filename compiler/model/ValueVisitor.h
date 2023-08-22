#ifndef COMPILER_MODEL_VALUEVISITOR_H_
#define COMPILER_MODEL_VALUEVISITOR_H_

#include <utility>

#define _VALUE_TYPES() \
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

namespace comp {

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

}  // namespace comp


#endif /* COMPILER_MODEL_VALUEVISITOR_H_ */
