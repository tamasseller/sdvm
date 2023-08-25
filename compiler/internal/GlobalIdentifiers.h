#ifndef COMPILER_REFERENCEEXRACTION_H_
#define COMPILER_REFERENCEEXRACTION_H_

#include "compiler/model/Class.h"
#include "compiler/model/Function.h"

#include <map>

namespace comp {

struct GlobalIdentifiers
{
	std::map<std::shared_ptr<Function>, size_t> functions;
	std::map<std::shared_ptr<Class>, size_t> classes;

	static GlobalIdentifiers gather(std::shared_ptr<Function> entryPoint);
};

} // namespace comp

#endif /* COMPILER_REFERENCEEXRACTION_H_ */
