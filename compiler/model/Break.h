#ifndef COMPILER_MODEL_BREAK_H_
#define COMPILER_MODEL_BREAK_H_

#include "Loop.h"

namespace comp {

struct Break: StatementBase<Break>
{
	const std::shared_ptr<Loop> loop;

	inline Break(std::shared_ptr<Loop> loop): loop(loop) {}
};

}  // namespace comp

#endif /* COMPILER_MODEL_BREAK_H_ */
