#ifndef COMPILER_CLASSDESC_H_
#define COMPILER_CLASSDESC_H_

#include "ValueType.h"

#include <vector>
#include <memory>

namespace comp {
namespace ast {

class ProgramObjectSet;

struct Class: std::enable_shared_from_this<Class>
{
	const std::shared_ptr<Class> base;
	std::vector<ValueType> fieldTypes;
	std::vector<ValueType> staticTypes;

	inline Class(std::shared_ptr<Class> base): base(base) {};

	std::string dump(const ProgramObjectSet& gi) const;
	std::string getReferenceForDump(const ProgramObjectSet& gi) const;
};

} // namespace ast
} // namespace comp

#endif /* COMPILER_CLASSDESC_H_ */
