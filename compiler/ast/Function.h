#ifndef COMPILER_FUNCTIONDESC_H_
#define COMPILER_FUNCTIONDESC_H_

#include "ValueType.h"
#include "meta/Statement.h"

namespace comp {
namespace ast {

class Block;
class ProgramObjectSet;

struct Function
{
	std::vector<ValueType> ret, args;
	std::shared_ptr<Block> body;

	Function(std::vector<ValueType> ret, std::vector<ValueType> args);

	std::string dump(const ProgramObjectSet& gi) const;
	std::string getReferenceForDump(const ProgramObjectSet& gi) const;
};

} // namespace ast
} // namespace comp

#endif /* COMPILER_FUNCTIONDESC_H_ */
