#ifndef COMPILER_FUNCTIONBUILDER_H_
#define COMPILER_FUNCTIONBUILDER_H_

#include "Type.h"
#include "RValue.h"

#include "program/Function.h"
#include "object/Storage.h"

#include <optional>

namespace comp {

class ProgramBuilder;

class FunctionBuilder
{
	friend class ProgramBuilder;

	std::optional<Type> retType;
	std::vector<Type> args;
	size_t nextLocal;
	std::vector<size_t> frameRefIndices;
	std::vector<prog::Instruction> code;

	int stackDepth = 0, maxStackDepth = 0;
	bool hasCall = false;

	void write(prog::Instruction isn);
	FunctionBuilder(const FunctionBuilder&) = delete;
	FunctionBuilder(FunctionBuilder&&) = delete;
	prog::Function operator()(std::vector<obj::Type>&);

public:
	FunctionBuilder(std::optional<Type> ret, std::vector<Type> args);

	RValue arg(size_t n);
	RValue addi(const RValue& a, const RValue& b);
	void ret(const RValue& v);
	void ret();
};

} //namespace comp

#endif /* COMPILER_FUNCTIONBUILDER_H_ */
