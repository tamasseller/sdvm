#include "ProgramBuilder.h"

using namespace comp;

ProgramBuilder::ProgramBuilder() {
	globalBuilder = type();
}

ProgramBuilder::Function ProgramBuilder::fun(std::optional<ValueType> ret, std::vector<ValueType> args)
{
	auto idx = functions.size();
	functions.push_back(std::make_unique<FunctionBuilder>(*this, ret, args));
	return {functions, idx};
}

ProgramBuilder::Class ProgramBuilder::type(std::optional<Handle<ClassBuilder>> base, bool isFrame)
{
	auto idx = types.size();
	types.push_back(std::make_unique<ClassBuilder>(idx, base.has_value() ? base->idx : std::optional<uint32_t>{}, isFrame));
	return {types, idx};
}

prog::Program ProgramBuilder::operator()()
{
	prog::Program ret;

	// Generate code and function meta-data
	std::transform(functions.cbegin(), functions.cend(), std::back_inserter(ret.functions), [&ret](const auto& f){ return (*f)();});

	// Generate runtime type info
	std::transform(types.cbegin(), types.cend(), std::back_inserter(ret.types), [&ret](const auto& t){ return (*t)();});

	return ret;
}

uint32_t ProgramBuilder::getFieldOffset(ClassBuilder::FieldHandle f) const
{
	auto baseSize = 0u;
	for(auto idx = f.typeIdx; types[f.typeIdx]->baseIdx; idx = *types[f.typeIdx]->baseIdx)
	{
		baseSize += types[idx]->size();
	}

	return baseSize + f.fieldIdx;
}
