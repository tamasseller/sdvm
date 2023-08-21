#ifndef COMPILER_MODEL_STATEMENT_H_
#define COMPILER_MODEL_STATEMENT_H_

#include "StatementVisitor.h"

namespace comp {

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

template<class Child>
struct StatementBase: Statement
{
	inline virtual void accept(const StatementVisitor& v) const override final {
		v.visit(*static_cast<const Child*>(this));
	}

	inline virtual ~StatementBase() = default;
};

}  // namespace comp


#endif /* COMPILER_MODEL_STATEMENT_H_ */
