#include "ProgramBuilder.h"

using namespace comp;

ProgramBuilder::Handle<FunctionBuilder> ProgramBuilder::fun(std::optional<Type> ret, std::vector<Type> args)
{
	auto idx = functions.size();
	functions.push_back(std::make_unique<FunctionBuilder>(ret, args));
	return {functions, idx};
}

ProgramBuilder::Handle<ClassBuilder> ProgramBuilder::type(std::optional<Handle<ClassBuilder>> base)
{
	auto idx = types.size();
	types.push_back(std::make_unique<ClassBuilder>(base.has_value() ? base->idx : std::optional<uint32_t>{}));
	return {types, idx};
}

prog::Program ProgramBuilder::operator()()
{
	prog::Program ret;

	// Add global object
	ret.types.push_back(obj::Type::empty);

	// Add classes
	std::transform(types.cbegin(), types.cend(), std::back_inserter(ret.types), [](const auto& t){ return (*t)();});

	// Add functions (and frame object types)
	std::transform(functions.cbegin(), functions.cend(), std::back_inserter(ret.functions), [&ret](const auto& f){ return (*f)(ret.types);});

	return ret;
}
