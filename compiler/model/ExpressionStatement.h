#ifndef COMPILER_MODEL_EXPRESSIONSTATEMENT_H_
#define COMPILER_MODEL_EXPRESSIONSTATEMENT_H_

#include "RValue.h"
#include "Statement.h"

namespace comp {

struct ExpressionStatement: StatementBase<ExpressionStatement>
{
	std::shared_ptr<RValue> val;
	inline ExpressionStatement(decltype(val) val): val(val) {}
};

}  // namespace comp

#endif /* COMPILER_MODEL_EXPRESSIONSTATEMENT_H_ */
