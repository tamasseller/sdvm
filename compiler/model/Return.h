#ifndef COMPILER_MODEL_RETURN_H_
#define COMPILER_MODEL_RETURN_H_

#include "Statement.h"
#include "RValue.h"

#include <vector>

namespace comp {

struct Return: StatementBase<Return>
{
	std::vector<std::shared_ptr<RValue>> value;

	inline Return(const std::vector<std::shared_ptr<RValue>> &value = {}): value(value) {}
	inline virtual ~Return() = default;
};

} // namespace comp

#endif /* COMPILER_MODEL_RETURN_H_ */
