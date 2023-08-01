#include "ClassBuilder.h"
#include "ProgramBuilder.h"

using namespace comp;

obj::TypeInfo ClassBuilder::operator()()
{
	std::vector<size_t> refOffs;

	for(auto i = 0u; i < fieldTypes.size(); i++)
	{
		if(!fieldTypes[i].kind == TypeKind::Value)
		{
			refOffs.push_back(i);
		}
	}

	return obj::TypeInfo(baseIdx.value_or(0), size(), refOffs, isFrame);
}
