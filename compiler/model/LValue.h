#ifndef COMPILER_MODEL_LVALUE_H_
#define COMPILER_MODEL_LVALUE_H_

#include "RValue.h"

namespace comp {

struct LValue: RValue {};

template<class Child>
struct LValueBase: ValueBase<Child, LValue> {};

} //namespace comp

#endif /* COMPILER_MODEL_LVALUE_H_ */
