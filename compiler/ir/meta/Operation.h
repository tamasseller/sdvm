#ifndef COMPILER_IR_META_OPERATION_H_
#define COMPILER_IR_META_OPERATION_H_

#include <utility>

namespace comp {
namespace ir {

#define _OPERATION_TYPES() \
		X(Copy) \
		X(Literal) \
		X(Unary) \
		X(Create) \
		X(LoadField) \
		X(StoreField) \
		X(LoadGlobal) \
		X(StoreGlobal) \
		X(Binary) \
		X(Call) \

#define X(n) class n;
_OPERATION_TYPES()
#undef X

struct OperationVisitor
{
#define X(n) virtual void visit(const n&) const = 0;
_OPERATION_TYPES()
#undef X

	inline virtual ~OperationVisitor() = default;
};

template<class C>
struct LambdaOperationVisitor: OperationVisitor
{
	C c;

	LambdaOperationVisitor(C&& c): c(std::forward<C>(c)) {}
	inline virtual ~LambdaOperationVisitor() = default;

#define X(n) virtual void visit(const n &v) const override final { c(v); };
_OPERATION_TYPES()
#undef X
};

struct Operation
{
	virtual void accept(const OperationVisitor& v) const = 0;

	template<class C>
	inline void accept(C&& c) const {
		const LambdaOperationVisitor<C> v(std::forward<C>(c));
		this->accept((const OperationVisitor&)v);
	}

	inline virtual ~Operation() = default;
};

template<class Child>
struct OperationBase: Operation
{
	inline virtual void accept(const OperationVisitor& v) const override final {
		v.visit(*static_cast<const Child*>(this));
	}

	inline virtual ~OperationBase() = default;
};

} // namespace ir
} // namespace comp

#endif /* COMPILER_IR_META_OPERATION_H_ */
