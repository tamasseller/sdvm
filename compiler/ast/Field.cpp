#include "Field.h"

#include "ProgramObjectSet.h"

using namespace comp;
using namespace comp::ast;

std::string Field::getReferenceForDump(const ProgramObjectSet& gi) const
{
	return "f" + std::to_string(index);
}

std::string StaticField::getReferenceForDump(const ProgramObjectSet& gi) const {
	return type->getReferenceForDump(gi) + "::s" + std::to_string(index);
}
