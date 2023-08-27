#include "ValueType.h"

#include "ProgramObjectSet.h"

using namespace comp::ast;

std::string ValueType::getReferenceForDump(const ProgramObjectSet &gi) const
{
	if(kind == TypeKind::Value)
	{
		switch(primitiveType) {
			case PrimitiveType::Integer: return "int";
			case PrimitiveType::Floating: return "float";
			case PrimitiveType::Logical: return "logical";
			case PrimitiveType::Native: return "native";
			default: return "???";
		}
	}
	else
	{
		assert(kind == TypeKind::Reference);
		return referenceType->getReferenceForDump(gi) + "*";
	}
}
