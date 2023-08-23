#ifndef COMPILER_MODEL_CONTINUE_H_
#define COMPILER_MODEL_CONTINUE_H_

#include "Loop.h"

namespace comp {

struct Continue: StatementBase<Continue>
{
	const std::shared_ptr<Loop> loop;

	inline Continue(std::shared_ptr<Loop> loop): loop(loop) {}
};

}  // namespace comp

#endif /* COMPILER_MODEL_CONTINUE_H_ */
