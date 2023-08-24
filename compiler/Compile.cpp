#include "builder/FunctionBuilder.h"

#include "ReferenceExraction.h"
#include "AstDump.h"

using namespace comp;

#include <iostream>

prog::Program comp::compile(std::shared_ptr<Function> data)
{
	prog::Program ret;

	auto rawReferences = gatherReferences(data);
	auto allRefs = summarizeReferences(rawReferences);
	auto fnIdxTable = assignIndices(allRefs.functions, data);
	auto globalClass = gatherStaticFields(allRefs.classes);
	auto classIdxTable = assignIndices(allRefs.classes, globalClass);

	for(const auto &c: allRefs.classes)
	{
		std::cout << dumpClassAst(classIdxTable, c) << std::endl;
	}

	for(const auto &i: allRefs.functions)
	{
		std::cout << dumpFunctionAst(classIdxTable, fnIdxTable, i) << std::endl;
	}

	return ret;
}
