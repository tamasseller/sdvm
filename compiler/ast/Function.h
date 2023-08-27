#ifndef COMPILER_FUNCTIONDESC_H_
#define COMPILER_FUNCTIONDESC_H_

#include "ValueType.h"
#include "Statements.h"

#include <optional>
#include <algorithm>

namespace comp {
namespace ast {

class ProgramObjectSet;

struct Function
{
	std::vector<ValueType> ret, args;
	std::shared_ptr<Block> body = std::make_shared<Block>();

	inline Function(std::vector<ValueType> ret, std::vector<ValueType> args): ret(ret), args(args) {}

	std::string dump(const ProgramObjectSet& gi) const;
	std::string getReferenceForDump(const ProgramObjectSet& gi) const;
};

} // namespace ast
} // namespace comp

#endif /* COMPILER_FUNCTIONDESC_H_ */
