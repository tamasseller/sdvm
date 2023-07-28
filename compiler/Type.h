#ifndef COMPILER_TYPE_H_
#define COMPILER_TYPE_H_

#include "object/Type.h"

namespace comp {

enum TypeKind {
	Reference, Value
};

enum ValueType {
	Integer, Floating, Logical, Native
};

struct Type {
	TypeKind kind;

	union {
		ValueType primitiveType;
		Type* referenceType;
	};

	static inline auto integer() { return Type{ .kind = TypeKind::Value, .primitiveType = ValueType::Integer}; }
};

} //namespace comp

#endif /* COMPILER_TYPE_H_ */
