#include "Compiler.h"
#include "AstDump.h"

#include <sstream>
#include <algorithm>

using namespace comp;

std::string Compiler::dumpAst()
{
	std::stringstream ss;

	const char* sep = "";

	std::map<size_t, std::shared_ptr<Class>> revClasses;
	std::transform(gi.classes.begin(), gi.classes.end(), std::inserter(revClasses, revClasses.end()), [](const auto &p){return std::make_pair(p.second, p.first);});
	for(const auto &c: revClasses)
	{
		ss << sep << dumpClassAst(gi, c.second);
		sep = "\n";
	}

	std::map<size_t, std::shared_ptr<Function>> revFunctions;
	std::transform(gi.functions.begin(), gi.functions.end(), std::inserter(revFunctions, revFunctions.end()), [](const auto &p){return std::make_pair(p.second, p.first);});
	for(const auto &i: revFunctions)
	{
		ss << sep << dumpFunctionAst(gi, i.second);
		sep = "\n";
	}

	const auto ret = ss.str();
	return ret;
}

prog::Program Compiler::compile()
{
	prog::Program ret;

	return ret;
}
