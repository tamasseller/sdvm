#include "Compile.h"
#include "AstDump.h"
#include "GlobalIdentifiers.h"

using namespace comp;

#include <iostream>

prog::Program comp::compile(std::shared_ptr<Function> entryPoint)
{
	prog::Program ret;

	const auto allRefs = GlobalIdentifiers::gather(entryPoint);

	for(const auto &c: allRefs.classes)
	{
		std::cout << dumpClassAst(allRefs, c.first) << std::endl;
	}

	for(const auto &i: allRefs.functions)
	{
		std::cout << dumpFunctionAst(allRefs, i.first) << std::endl;
	}

	return ret;
}
