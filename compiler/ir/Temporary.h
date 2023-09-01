#ifndef COMPILER_IR_TEMPORARY_H_
#define COMPILER_IR_TEMPORARY_H_

#include "compiler/ast/ValueType.h"

namespace comp {
namespace ir {

/*
 * TAC value (constant or variable).
 */
struct Temporary
{
	const ast::ValueType type;
	virtual bool isConstant() = 0;

	inline Temporary(ast::ValueType type): type(type) {}
	inline virtual ~Temporary() = default;
};

/*
 * A proto-register.
 */
struct Variable: Temporary
{
	inline virtual bool isConstant() {
		return false;
	}

	inline Variable(ast::ValueType type): Temporary(type) {}
};

/*
 * A value that is fully known at compilation time.
 */
struct Constant: Temporary
{
	const int value;

	inline virtual bool isConstant() {
		return true;
	}

	inline Constant(ast::ValueType type, int value): Temporary(type), value(value) {}
};

} // namespace ir
} // namespace comp


#endif /* COMPILER_IR_TEMPORARY_H_ */
