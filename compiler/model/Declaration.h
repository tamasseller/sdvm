#ifndef COMPILER_MODEL_DECLARATION_H_
#define COMPILER_MODEL_DECLARATION_H_

#include "Statement.h"
#include "Local.h"
#include "RValue.h"

namespace comp {

struct Declaration: StatementBase<Declaration>
{
	const std::shared_ptr<Local> local;
	const std::shared_ptr<RValue> initializer;

	inline Declaration(std::shared_ptr<Local> local, std::shared_ptr<RValue> initializer): local(local), initializer(initializer) {}
	inline virtual ~Declaration() = default;
};

}  // namespace comp

#endif /* COMPILER_MODEL_DECLARATION_H_ */
