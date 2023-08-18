#ifndef COMPILER_MODEL_DECLARATION_H_
#define COMPILER_MODEL_DECLARATION_H_

#include "Statement.h"
#include "Local.h"

namespace comp {

struct Declaration: Statement
{
	const std::shared_ptr<Local> local;

	inline Declaration(std::shared_ptr<Local> local): local(local) {}
};

}  // namespace comp

#endif /* COMPILER_MODEL_DECLARATION_H_ */
