#ifndef COMPILER_VALUETYPE_H_
#define COMPILER_VALUETYPE_H_

#include <cstdint>

namespace comp {

enum TypeKind {
	Value, Reference, Dynamic
};

enum PrimitiveType {
	Integer, Floating, Logical, Native
};

struct ValueType
{
	TypeKind kind;

	union {
		PrimitiveType primitiveType;
		uint32_t referencedTypeIndex;
	};

	static inline auto integer() { return ValueType{ .kind = TypeKind::Value, .primitiveType = PrimitiveType::Integer}; }
	static inline auto floating() { return ValueType{ .kind = TypeKind::Value, .primitiveType = PrimitiveType::Floating}; }
	static inline auto logical() { return ValueType{ .kind = TypeKind::Value, .primitiveType = PrimitiveType::Logical}; }
	static inline auto native() { return ValueType{ .kind = TypeKind::Value, .primitiveType = PrimitiveType::Native}; }
	static inline auto reference(uint32_t idx) { return ValueType{ .kind = TypeKind::Reference, .referencedTypeIndex = idx}; }
	static inline auto dynamic() { return ValueType{ .kind = TypeKind::Dynamic}; }
};

} //namespace comp

#endif /* COMPILER_VALUETYPE_H_ */
