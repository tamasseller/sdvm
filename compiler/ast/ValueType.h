#ifndef COMPILER_VALUETYPE_H_
#define COMPILER_VALUETYPE_H_

#include <cstdint>
#include <memory>
#include <string>

namespace comp {
namespace ast {

class Class;
class ProgramObjectSet;

enum TypeKind {
	Value, Reference
};

enum PrimitiveType {
	Integer, Floating, Logical, Native
};

struct ValueType
{
	TypeKind kind;
	PrimitiveType primitiveType;
	const std::shared_ptr<Class> referenceType;

	static inline auto primitive(PrimitiveType primitiveType) { return ValueType{ .kind = TypeKind::Value, .primitiveType = primitiveType}; }
	static inline auto integer() { return primitive(PrimitiveType::Integer); }
	static inline auto floating() { return primitive(PrimitiveType::Floating); }
	static inline auto logical() { return primitive(PrimitiveType::Logical); }
	static inline auto native() { return primitive(PrimitiveType::Native); }
	static inline auto reference(std::shared_ptr<Class> referenceType) { return ValueType{ .kind = TypeKind::Reference, .referenceType = referenceType}; }

	std::string getReferenceForDump(const ProgramObjectSet &gi) const;
};

} // namespace ast
} //namespace comp

#endif /* COMPILER_VALUETYPE_H_ */
