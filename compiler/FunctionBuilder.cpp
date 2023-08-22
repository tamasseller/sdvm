#include "FunctionBuilder.h"

#include "ReferenceExraction.h"

using namespace comp;

#include <iostream>

static inline std::string typeName(ValueType vt, const std::map<std::shared_ptr<Class>, size_t> &classIdxTable)
{
	if(vt.kind == TypeKind::Value)
	{
		switch(vt.primitiveType) {
			case PrimitiveType::Integer: return "int";
			case PrimitiveType::Floating: return "float";
			case PrimitiveType::Logical: return "logical";
			case PrimitiveType::Native: return "native";
			default: return "???";
		}
	}
	else
	{
		assert(vt.kind == TypeKind::Reference);
		return "c" + std::to_string(classIdxTable.find(vt.referenceType)->second);
	}
}

prog::Program FunctionBuilder::compile() const
{
	prog::Program ret;

	auto rawReferences = gatherReferences(data);
	auto allRefs = summarizeReferences(rawReferences);
	auto fnIdxTable = assignIndices(allRefs.functions, data);
	auto globalClass = gatherStaticFields(allRefs.classes);
	auto classIdxTable = assignIndices(allRefs.classes, globalClass);

	for(const auto &i: rawReferences)
	{
		auto from = fnIdxTable.find(i.first)->second;
		for(const auto &j: i.second.functions)
		{
			auto to = fnIdxTable.find(j)->second;
			std::cout << "f" << from << " -> f" << to << ";" << std::endl;
		}
	}

	for(const auto &c: allRefs.classes)
	{
		auto cidx = classIdxTable.find(c)->second;
		std::cout << "c" << cidx << " {";

		const char *sep = "";
		for(auto i = 0u; i < c->staticTypes.size(); i++)
		{
			std::cout << sep << "s" << i << ": " << typeName(c->staticTypes[i], classIdxTable);
			sep = ", ";
		}

		for(auto i = 0u; i < c->fieldTypes.size(); i++)
		{
			std::cout << sep << i << ": " << typeName(c->fieldTypes[i], classIdxTable);
			sep = ", ";
		}

		std::cout << "}" << std::endl;
	}

	return ret;
}
