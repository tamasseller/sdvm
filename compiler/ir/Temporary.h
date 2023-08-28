#ifndef COMPILER_IR_TEMPORARY_H_
#define COMPILER_IR_TEMPORARY_H_

#include "compiler/ast/ValueType.h"

namespace comp {
namespace ir {

/*
 * A proto-register
 */
struct Temporary
{
	const ast::ValueType type;
	inline Temporary(ast::ValueType type): type(type) {}
};

} // namespace ir
} // namespace comp


#endif /* COMPILER_IR_TEMPORARY_H_ */
