#ifndef COMPILER_FUNCTIONBUILDER_H_
#define COMPILER_FUNCTIONBUILDER_H_

#include "Handle.h"
#include "ClassBuilder.h"

#include "RValue.h"
#include "ValueType.h"

#include "program/Function.h"
#include "object/Storage.h"

#include <optional>

namespace comp {

class ProgramBuilder;

class FunctionBuilder
{
	friend class ProgramBuilder;

	ProgramBuilder& pb;
	std::optional<ValueType> retType;
	std::vector<ValueType> argTypes;
	std::vector<std::function<void(CodeWriter&)>> code;

	FunctionBuilder(const FunctionBuilder&) = delete;
	FunctionBuilder(FunctionBuilder&&) = delete;
	prog::Function operator()();

	Handle<ClassBuilder> frameBuilder;

public:
	FunctionBuilder(ProgramBuilder& pb, std::optional<ValueType> ret, std::vector<ValueType> args);

	template<class... Args>
	auto addLocal(Args&&... args) {
		return frameBuilder->addField(std::forward<Args>(args)...);
	}

	RValue arg(size_t n);
	RValue create(Handle<ClassBuilder> t);
	RValue addi(const RValue& a, const RValue& b);
	void setLocal(const ClassBuilder::FieldHandle &l, const RValue& b);
	void ret(const RValue& v);
	void ret();
};

} //namespace comp

#endif /* COMPILER_FUNCTIONBUILDER_H_ */
