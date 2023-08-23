#ifndef COMPILER_MODEL_TESTBEFORELOOP_H_
#define COMPILER_MODEL_TESTBEFORELOOP_H_

#include "Block.h"

namespace comp {

struct Loop: StatementBase<Loop>
{
	const std::shared_ptr<Block> body = std::make_shared<Block>();
};

}  // namespace comp

#endif /* COMPILER_MODEL_TESTBEFORELOOP_H_ */
