#ifndef INSTRUCTION_H_
#define INSTRUCTION_H_

#include "object/Value.h"

#include <cstdint>
#include <cstddef>

namespace prog {

enum class StackReadType { // TODO move into Instruction and make private
	Zero, One, Two, ArgDep
};

#define INSTRUCTION_LIST() \
	X(NewObject,     StackReadType::Zero,   true)  /* push(create(arg)) */ \
	X(PushLiteral,   StackReadType::Zero,   true)  /* push(arg) */ \
	X(ReadField,     StackReadType::One,    true)  /* push(read(pop(), arg)) */ \
	X(ReadLocal,     StackReadType::Zero,   true)  /* push(read(<local>, arg)) */ \
	X(ReadStatic,    StackReadType::Zero,   true)  /* push(read(<static>, arg)) */ \
	X(WriteField,  	 StackReadType::Two,    false) /* write(pop(), arg, pop()) */ \
	X(WriteLocal,    StackReadType::One,    false) /* write(<local>, arg, pop()) */ \
	X(WriteStatic,   StackReadType::One,    false) /* write(<static>, arg, pop()) */ \
	X(Unary,         StackReadType::One,    true)  /* push(op<arg>(pop()) */ \
	X(Binary,        StackReadType::Two,    true)  /* push(op<arg>(pop(), pop()) */ \
	X(Jump,          StackReadType::Zero,   false) /* goto(arg) */ \
	X(Cond,          StackReadType::One,    false) /* if(cond(pop())) { goto(arg); } */ \
	X(Ret,           StackReadType::Zero,   false) /* ret() */ \
	X(RetVal,        StackReadType::One,    false) /* ret(pop()) */ \
	X(CallV,         StackReadType::ArgDep, false) /* call(pop(), pop()... * args) */ \
	X(Call,	         StackReadType::ArgDep, true)  /* call(pop(), pop()... * args) */

	// TODO primitive typecasts
	// TODO instanceof
	// TODO throw
	// TODO native call

struct Instruction
{
	enum class Operation
	{
#define X(name, popCount, doesWb) name,
		INSTRUCTION_LIST()
#undef X
	};

	enum class BinaryOpType {
		AddI, MulI, SubI, DivI,
		EqI, NeI, LtI, GtI, LeI, GeI,
		Mod, ShlI, ShrI, ShrU, AndI, OrI, XorI,
		AddF, MulF, SubF, DivF,
		EqF, NeF, LtF, GtF, LeF, GeF,
		AndL, OrL
	};

	enum class UnaryOpType {
		Not, BitwiseNegate
	};

	union Argument
	{
		obj::Value literal;
		uint32_t index; // type/field/jump offset or argument count;
		BinaryOpType binOp;
		UnaryOpType unOp;

		constexpr Argument() = default;
		constexpr Argument(const Argument&) = default;
		constexpr Argument(const obj::Value &literal): literal(literal) {}
		constexpr Argument(uint32_t index): index(index) {}
		constexpr Argument(BinaryOpType binOp): binOp(binOp) {}
		constexpr Argument(UnaryOpType unOp): unOp(unOp) {}
	};

	Operation op;
	Argument arg;

	static inline constexpr size_t getPopCountOf(StackReadType srt, size_t arg)
	{
		switch(srt)
		{
			case StackReadType::Zero:   return 0;
			case StackReadType::One:    return 1;
			case StackReadType::Two:    return 2;
			default:
			case StackReadType::ArgDep: return arg;
		}
	}

	inline constexpr size_t popCount(size_t n = 0) const
	{
		switch(op)
		{

#define X(name, popCount, doesWb) case Operation:: name: return getPopCountOf(popCount, n);
		INSTRUCTION_LIST()
#undef X
		}

		return -1u;
	}

	inline constexpr bool doesWriteBack() const
	{
		switch(op)
		{
#define X(name, popCount, doesWb) case Operation:: name: return doesWb;
		INSTRUCTION_LIST()
#undef X
		default: return false;
		}
	}

	int stackBalance() const {
		return (doesWriteBack() ? 1 : 0) - popCount(arg.index);
	}

	inline Instruction(): arg(0) {} // @suppress("Class members should be properly initialized")

	inline Instruction(const Instruction&) = default;

	constexpr inline Instruction(Operation op, Argument arg): op(op), arg(arg) {}

	static constexpr inline Instruction literal(obj::Value v) { return {Operation::PushLiteral, v}; }
	static constexpr inline Instruction readLocal(uint32_t offset) { return {Operation::ReadLocal, offset}; }
	static constexpr inline Instruction readStatic(uint32_t offset) { return {Operation::ReadStatic, offset}; }
	static constexpr inline Instruction newObject(uint32_t typeIndex) { return {Operation::NewObject, typeIndex}; }
	static constexpr inline Instruction callNoRet(uint32_t count) { return {Operation::CallV, count}; }
	static constexpr inline Instruction callWithRet(uint32_t count) { return {Operation::Call, count}; }
	static constexpr inline Instruction ret() { return {Operation::Ret, 0u}; }
	static constexpr inline Instruction retVal() { return {Operation::RetVal, 0u}; }
	static constexpr inline Instruction jump(uint32_t target) { return {Operation::Jump, target}; }
	static constexpr inline Instruction cond(uint32_t target) { return {Operation::Cond, target}; }
	static constexpr inline Instruction writeLocal(uint32_t offset) { return {Operation::WriteLocal, offset}; }
	static constexpr inline Instruction writeStatic(uint32_t offset) { return {Operation::WriteStatic, offset}; }
	static constexpr inline Instruction readField(uint32_t offset) { return {Operation::ReadField, offset}; }
	static constexpr inline Instruction unary(UnaryOpType op) { return {Operation::Unary, op}; }
	static constexpr inline Instruction binary(BinaryOpType op) { return {Operation::Binary, op}; }
	static constexpr inline Instruction writeField(uint32_t offset) { return {Operation::WriteField, offset}; }
};

} //namespace prog

#endif /* INSTRUCTION_H_ */
