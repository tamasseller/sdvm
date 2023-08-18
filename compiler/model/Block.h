#ifndef COMPILER_MODEL_BLOCK_H_
#define COMPILER_MODEL_BLOCK_H_

#include "Statement.h"

#include <memory>
#include <vector>

namespace comp {

struct Block: Statement
{
	std::vector<std::shared_ptr<Statement>> stmts;
};

}  // namespace comp

#endif /* COMPILER_MODEL_BLOCK_H_ */
