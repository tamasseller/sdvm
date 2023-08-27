#ifndef COMPILER_INTERNAL_TACIFY_H_
#define COMPILER_INTERNAL_TACIFY_H_

#include "compiler/model/BasicBlock.h"

namespace comp {

std::vector<std::shared_ptr<BasicBlock>> tacify(std::shared_ptr<Function> f);

}  // namespace comp

#endif /* COMPILER_INTERNAL_TACIFY_H_ */
