#ifndef COMPILER_MODEL_STATEMENT_H_
#define COMPILER_MODEL_STATEMENT_H_

#include <vector>
#include <memory>

namespace comp {

#define _STATEMENT_TYPES() \
	X(ExpressionStatement) \
	X(Conditional) \
	X(Declaration) \
	X(Continue) \
	X(Return) \
	X(Block) \
	X(Break) \
	X(Loop) \

#define X(n) class n;
_STATEMENT_TYPES()
#undef X

struct StatementVisitor
{
#define X(n) virtual void visit(const n&) const = 0;
_STATEMENT_TYPES()
#undef X

	inline virtual ~StatementVisitor() = default;
};

template<class C>
struct LambdaStatementVisitor: StatementVisitor
{
	C c;

	LambdaStatementVisitor(C&& c): c(std::forward<C>(c)) {}
	inline virtual ~LambdaStatementVisitor() = default;

#define X(n) virtual void visit(const n &v) const override final { c(v); };
_STATEMENT_TYPES()
#undef X
};

struct Statement
{
	virtual void accept(const StatementVisitor& v) const = 0;

	template<class C>
	inline void accept(C&& c) const {
		const LambdaStatementVisitor<C> v(std::forward<C>(c));
		this->accept((const StatementVisitor&)v);
	}

	inline virtual ~Statement() = default;
};

}  // namespace comp

#endif /* COMPILER_MODEL_STATEMENT_H_ */
