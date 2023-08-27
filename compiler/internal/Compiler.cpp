#include "Compiler.h"
#include "AstDump.h"

#include <sstream>
#include <algorithm>

using namespace comp;

struct ProperlyOrderedReferences
{
	std::vector<std::shared_ptr<Class>> classes;
	std::vector<std::shared_ptr<Function>> functions;

	static ProperlyOrderedReferences from(const GlobalIdentifiers& gi)
	{
		ProperlyOrderedReferences ret;

		std::map<size_t, std::shared_ptr<Class>> revClasses;
		std::transform(gi.classes.begin(), gi.classes.end(), std::inserter(revClasses, revClasses.end()), [](const auto &p){return std::make_pair(p.second, p.first);});
		std::transform(revClasses.begin(), revClasses.end(), std::back_inserter(ret.classes), [](const auto &p){return p.second;});

		std::map<size_t, std::shared_ptr<Function>> revFunctions;
		std::transform(gi.functions.begin(), gi.functions.end(), std::inserter(revFunctions, revFunctions.end()), [](const auto &p){return std::make_pair(p.second, p.first);});
		std::transform(revFunctions.begin(), revFunctions.end(), std::back_inserter(ret.functions), [](const auto &p){return p.second;});
		return ret;
	}
};

template<class C>
std::string join(C& c, const char* seperator = "\n")
{
	const char* sep = "";
	std::stringstream ss;

	for(const auto &v: c)
	{
		ss << sep << v;
		sep = seperator;
	}

	const auto ret = ss.str();
	return ret;
}

std::string Compiler::dumpAst()
{
	const auto por = ProperlyOrderedReferences::from(gi);
	std::vector<std::string> parts;

	std::transform(por.classes.begin(), por.classes.end(), std::back_inserter(parts), [&](const auto &c){return dumpClassAst(gi, c);});
	std::transform(por.functions.begin(), por.functions.end(), std::back_inserter(parts), [&](const auto &f){return dumpFunctionAst(gi, f);});

	return join(parts);
}

std::string Compiler::dumpTac()
{
	const auto por = ProperlyOrderedReferences::from(gi);
	std::vector<std::string> parts;

	std::transform(por.classes.begin(), por.classes.end(), std::back_inserter(parts), [&](const auto &c){return dumpClassAst(gi, c);});
	std::transform(por.functions.begin(), por.functions.end(), std::back_inserter(parts), [&](const auto &f){return dumpCfg(gi, f);});

	return join(parts);
}

prog::Program Compiler::compile()
{
	prog::Program ret;

	return ret;
}
