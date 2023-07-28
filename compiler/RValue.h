#ifndef COMPILER_RVALUE_H_
#define COMPILER_RVALUE_H_

#include "Type.h"

#include <functional>

namespace comp {

struct RValue {
	Type type;
	std::function<void()> manifest;
};

} //namespace comp

#endif /* COMPILER_RVALUE_H_ */
