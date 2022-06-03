#ifndef VM_INTERPRETER_H_
#define VM_INTERPRETER_H_

#include "Program.h"

std::vector<uint32_t> interpret(const Program& p, const std::vector<uint32_t> &args, size_t stackSize = 4096);

#endif /* VM_INTERPRETER_H_ */
