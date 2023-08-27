#include "Compiler.h"

#include <sstream>
#include <algorithm>

using namespace comp;
using namespace comp::ast;

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
	std::vector<std::string> parts;

	std::transform(gi.classes.begin(), gi.classes.end(), std::back_inserter(parts), [&](const auto &c){return c->dump(gi);});
	std::transform(gi.functions.begin(), gi.functions.end(), std::back_inserter(parts), [&](const auto &f){return f->dump(gi);});

	return join(parts);
}

//std::string Compiler::dumpTac()
//{
//	std::vector<std::string> parts;
//
//	std::transform(gi.classes.begin(), gi.classes.end(), std::back_inserter(parts), [&](const auto &c){return c->dump(gi);});
//	std::transform(gi.functions.begin(), gi.functions.end(), std::back_inserter(parts), [&](const auto &f){return dumpCfg(gi, f);});
//
//	return join(parts);
//}

prog::Program Compiler::compile()
{
	prog::Program ret;

	return ret;
}