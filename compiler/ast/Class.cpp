#include "Class.h"

#include "ProgramObjectSet.h"

#include <sstream>

using namespace comp::ast;

std::string Class::getReferenceForDump(const ProgramObjectSet& gi) const {
	return std::string("c") + std::to_string(gi.getClassIndex(this));
}

std::string Class::dump(const ProgramObjectSet& gi) const
{
	std::stringstream ss;
	ss << "struct " << getReferenceForDump(gi) << std::endl;
	ss << "{" << std::endl;

	for(auto i = 0u; i < staticTypes.size(); i++) {
		ss << "  static " << staticTypes[i].getReferenceForDump(gi) << " s" << i << ";" << std::endl;
	}

	for(auto i = 0u; i < fieldTypes.size(); i++) {
		ss << "  " << fieldTypes[i].getReferenceForDump(gi) << " f" << i << ";" << std::endl;
	}

	ss << "}" << ";" << std::endl;
	const auto ret = ss.str();
	return ret;
}
