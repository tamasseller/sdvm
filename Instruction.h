#ifndef INSTRUCTION_H_
#define INSTRUCTION_H_

#include <stdint.h>

enum class WriteBackType {
	None, Val, Ref, Either
};

enum class StackReadType {
	Zero, One, Two, ArgDep
};

#define INSTRUCTION_LIST() \
	X(NewObject,     StackReadType::Zero,    WriteBackType::Ref)    /* push(create(arg)) */ \
	X(PushLiteral,   StackReadType::Zero,    WriteBackType::Val)    /* push(arg) */ \
	X(ReadValField,  StackReadType::One,     WriteBackType::Val)    /* push(read(pop(), arg)) */ \
	X(ReadRefField,  StackReadType::One,     WriteBackType::Ref)    /* push(read(pop(), arg)) */ \
	X(ReadValLocal,  StackReadType::Zero,    WriteBackType::Val)    /* push(read(<local>, arg)) */ \
	X(ReadRefLocal,  StackReadType::Zero,    WriteBackType::Ref)    /* push(read(<local>, arg)) */ \
	X(ReadValStatic, StackReadType::Zero,    WriteBackType::Val)    /* push(read(<static>, arg)) */ \
	X(ReadRefStatic, StackReadType::Zero,    WriteBackType::Ref)    /* push(read(<static>, arg)) */ \
	X(WriteField,  	 StackReadType::Two,     WriteBackType::None)   /* write(pop(), arg, pop()) */ \
	X(WriteLocal,    StackReadType::One,     WriteBackType::None)   /* write(<local>, arg, pop()) */ \
	X(WriteStatic,   StackReadType::One,     WriteBackType::None)   /* write(<static>, arg, pop()) */ \
	X(Unary,         StackReadType::One,     WriteBackType::Val)    /* push(op<arg>(pop()) */ \
	X(Binary,        StackReadType::Two,     WriteBackType::Val)    /* push(op<arg>(pop(), pop()) */ \
	X(Jump,          StackReadType::Zero,    WriteBackType::None)   /* goto(arg) */ \
	X(Cond,          StackReadType::One,     WriteBackType::None)   /* if(cond(pop())) { goto(arg); } */ \
	X(Ret,           StackReadType::Zero,    WriteBackType::None)   /* ret() */ \
	X(RetVal,        StackReadType::One,     WriteBackType::None)   /* ret(pop()) */ \
	X(RetRef,        StackReadType::One,     WriteBackType::None)   /* ret(pop()... * args) */ \
	X(CallV,         StackReadType::ArgDep,  WriteBackType::None)   /* call(pop(), pop()... * args) */ \
	X(Call,	         StackReadType::ArgDep,  WriteBackType::Either) /* call(pop(), pop()... * args) */

struct Instruction
{
	enum class Operation
	{
#define X(name, popCount, wbType) name,
		INSTRUCTION_LIST()
#undef X
	};

	enum class BinaryOpType {
		AddI, MulI, SubI, DivI, Mod,
		ShlI, ShrI, ShrU, AndI, OrI, XorI,
		EqI, NeI, LtI, GtI, LeI, GeI,
		AddF, MulF, SubF, DivF,
		EqF, NeF, LtF, GtF, LeF, GeF,
		And, Or, Xor,
	};

	enum class UnaryOpType {
		Not, BitwiseNegate
	};

	union Argument
	{
		Storage::Value literal;
		const Type* type;
		uint32_t jumpTarget;
		uint32_t fieldOffset;
		uint32_t argumentCount;
		BinaryOpType binOp;
		UnaryOpType unOp;

		constexpr Argument() = default;
		constexpr Argument(const Argument&) = default;
		constexpr Argument(const Storage::Value &literal): literal(literal) {}
		constexpr Argument(const Type* type): type(type) {}
		constexpr Argument(uint32_t jumpTarget): jumpTarget(jumpTarget) {}
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

#define X(name, popCount, wbType) case Operation:: name: return getPopCountOf(popCount, n);
		INSTRUCTION_LIST()
#undef X
		}

		return -1u;
	}

	inline constexpr bool doesValueWriteBack() const
	{
		switch(op)
		{
#define X(name, popCount, wbType) case Operation:: name: return wbType == WriteBackType::Val;
		INSTRUCTION_LIST()
#undef X
		default: return false;
		}
	}

	inline constexpr bool doesReferenceWriteBack() const
	{
		switch(op)
		{
#define X(name, popCount, wbType) case Operation:: name: return wbType == WriteBackType::Ref;
		INSTRUCTION_LIST()
#undef X
		default: return false;
		}
	}

	inline constexpr bool doesNotWriteBack() const
	{
		switch(op)
		{
#define X(name, popCount, wbType) case Operation:: name: return wbType == WriteBackType::None;
		INSTRUCTION_LIST()
#undef X
		default: return false;
		}
	}

	int stackBalance() const {
		return (doesNotWriteBack() ? 0 : 1) - popCount(arg.argumentCount);
	}

	inline Instruction() = default; // @suppress("Class members should be properly initialized")

	inline Instruction(const Instruction&) = default;

	constexpr inline Instruction(Operation op, Argument arg): op(op), arg(arg) {}

	static constexpr inline Instruction literal(Storage::Value v) {
		return {Operation::PushLiteral, v};
	}

	static constexpr inline Instruction readValueLocal(uint32_t offset) {
		return {Operation::ReadValLocal, offset};
	}

	static constexpr inline Instruction readRefenceLocal(uint32_t offset) {
		return {Operation::ReadRefLocal, offset};
	}

	static constexpr inline Instruction readValueStatic(uint32_t offset) {
		return {Operation::ReadValStatic, offset};
	}

	static constexpr inline Instruction readRefenceStatic(uint32_t offset) {
		return {Operation::ReadRefStatic, offset};
	}

	static constexpr inline Instruction newObject(const Type* type) {
		return {Operation::NewObject, type};
	}

	static constexpr inline Instruction callNoRet(uint32_t count) {
		return {Operation::CallV, count};
	}

	static constexpr inline Instruction callWithRet(uint32_t count) {
		return {Operation::Call, count};
	}

	static constexpr inline Instruction ret() {
		return {Operation::Ret, 0u};
	}

	static constexpr inline Instruction retRef() {
		return {Operation::RetRef, 0u};
	}

	static constexpr inline Instruction retVal() {
		return {Operation::RetVal, 0u};
	}

	static constexpr inline Instruction jump(uint32_t target) {
		return {Operation::Jump, target};
	}

	static constexpr inline Instruction cond(uint32_t target) {
		return {Operation::Cond, target};
	}

	static constexpr inline Instruction writeLocal(uint32_t offset) {
		return {Operation::WriteLocal, offset};
	}

	static constexpr inline Instruction writeStatic(uint32_t offset) {
		return {Operation::WriteStatic, offset};
	}

	static constexpr inline Instruction readValueField(uint32_t offset) {
		return {Operation::ReadValField, offset};
	}

	static constexpr inline Instruction readReferenceField(uint32_t offset) {
		return {Operation::ReadRefField, offset};
	}

	static constexpr inline Instruction unary(UnaryOpType op) {
		return {Operation::Unary, op};
	}

	static constexpr inline Instruction binary(BinaryOpType op) {
		return {Operation::Binary, op};
	}

	static constexpr inline Instruction writeField(uint32_t offset) {
		return {Operation::WriteField, offset};
	}
};

#endif /* INSTRUCTION_H_ */
