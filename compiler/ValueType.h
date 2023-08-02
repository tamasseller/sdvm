#ifndef COMPILER_VALUETYPE_H_
#define COMPILER_VALUETYPE_H_

#include <cstdint>
#include <memory>

namespace comp {

class ClassDesc;

enum TypeKind {
	Value, Reference, Dynamic
};

enum PrimitiveType {
	Integer, Floating, Logical, Native
};

struct ValueType
{
	TypeKind kind;
	PrimitiveType primitiveType;
	std::shared_ptr<ClassDesc> referenceType;

	static inline auto primitive(PrimitiveType primitiveType) { return ValueType{ .kind = TypeKind::Value, .primitiveType = primitiveType}; }
	static inline auto integer() { return primitive(PrimitiveType::Integer); }
	static inline auto floating() { return primitive(PrimitiveType::Floating); }
	static inline auto logical() { return primitive(PrimitiveType::Logical); }
	static inline auto native() { return primitive(PrimitiveType::Native); }
	static inline auto reference(std::shared_ptr<ClassDesc> referenceType) { return ValueType{ .kind = TypeKind::Reference, .referenceType = referenceType}; }
	static inline auto dynamic() { return ValueType{ .kind = TypeKind::Dynamic}; }
};

} //namespace comp

#endif /* COMPILER_VALUETYPE_H_ */
