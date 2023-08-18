#ifndef COMPILER_RVALUE_H_
#define COMPILER_RVALUE_H_

#include "ValueType.h"

#include "assert.h"

#include <functional>
#include <vector>

namespace comp {

struct RValue
{
	virtual ValueType getType() = 0;
	inline virtual ~RValue() = default;
};

} //namespace comp

#endif /* COMPILER_RVALUE_H_ */
