#ifndef COMPILER_FUNCTIONDESC_H_
#define COMPILER_FUNCTIONDESC_H_

#include "ValueType.h"
#include "Block.h"

#include <optional>
#include <algorithm>

namespace comp {

class Line;

struct Function
{
	std::vector<ValueType> ret, args;
	std::shared_ptr<Block> body = std::make_shared<Block>();

	inline Function(std::vector<ValueType> ret, std::vector<ValueType> args): ret(ret), args(args) {}
};

}  // namespace comp

#endif /* COMPILER_FUNCTIONDESC_H_ */
