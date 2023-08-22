#ifndef COMPILER_MODEL_STATEMENTVISITOR_H_
#define COMPILER_MODEL_STATEMENTVISITOR_H_

#include <utility>

#define _STATEMENT_TYPES() \
	X(ExpressionStatement) \
	X(Conditional) \
	X(Declaration) \
	X(Return) \
	X(Block) \
	X(Set) \

namespace comp {

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

}  // namespace comp

#endif /* COMPILER_MODEL_STATEMENTVISITOR_H_ */
