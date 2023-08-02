#ifndef COMPILER_LINE_H_
#define COMPILER_LINE_H_

#include "Field.h"
#include "ClassDesc.h"
#include "FunctionDesc.h"

#include "program/Instruction.h"

namespace comp {

class Line
{
	inline Line(prog::Instruction isn): kind(Kind::Instruction), isn(isn) {}

public:
	enum class Kind {
		Instruction,
		Label
	};

	Kind kind;
	prog::Instruction isn;
	std::shared_ptr<ClassDesc> type;
	Field field;
	std::shared_ptr<FunctionDesc> callee;

	inline Line(prog::Instruction::BinaryOpType op): Line(prog::Instruction::binary(op)) {}

	static inline Line ret(bool hasValue) {
		return {hasValue ? prog::Instruction::retVal() : prog::Instruction::ret()};
	}

	static inline Line readLocal(Field f) {
		Line ret(prog::Instruction::readLocal(f.index));
		ret.field = f;
		return ret;
	}

	std::optional<prog::Instruction> operator()() const
	{
		if(kind == Kind::Instruction) {
			return isn;
		}

		return {};
	}
};

} //namespace comp

#endif /* COMPILER_LINE_H_ */
