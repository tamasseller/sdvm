#ifndef COMPILER_COMPILE_H_
#define COMPILER_COMPILE_H_

#include "compiler/model/Function.h"

#include "program/Program.h"

namespace comp {

prog::Program compile(std::shared_ptr<Function> entryPoint);

} // namespace comp

#endif /* COMPILER_COMPILE_H_ */
