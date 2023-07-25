#ifndef INSTRUCTION_H_
#define INSTRUCTION_H_

#include <stdint.h>

struct Instruction
{
	enum class Operation
	{
														//										pop	     push
		PushLiteral, 									// push(arg)							0        1
		ReadLocal, 	 									// push(read(<local>, arg))				0        1
		ReadStatic, 	 								// push(read(<static>, arg))			0        1
		NewObject,										// push(create(arg))					0        1
		Call,											// call(pop(), pop()... * args);		0 (args) 0
		Ret,											// ret(pop()... * args);				0 (args) 0
		Jump, 											// goto(arg);							0        0
		Cond,											// if(cond(pop())) { goto(arg); }		1        0
		WriteLocal,  									// write(<local>, arg, pop()))			1        0
		WriteStatic,  									// write(<static>, arg, pop()))			1        0
		ReadField, 	 									// push(read(pop(), arg))				1        1
		Unary,											// push(op<arg>(pop()))					1        1
		Binary,											// push(op<arg>(pop(), pop()))			2        1
		WriteField,  									// write(pop(), arg, pop()))			2        0
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

	inline constexpr size_t popCount() const
	{
		switch(op)
		{
			case Operation::PushLiteral:
			case Operation::ReadLocal:
			case Operation::ReadStatic:
			case Operation::NewObject:
			case Operation::Call:
			case Operation::Ret:
			case Operation::Jump:
				return 0u;

			case Operation::Cond:
			case Operation::WriteLocal:
			case Operation::WriteStatic:
			case Operation::ReadField:
			case Operation::Unary:
				return 1u;

			case Operation::Binary:
			case Operation::WriteField:
				return 2u;
		}

		return -1u;
	}

	inline constexpr bool doesWriteBack() const
	{
		switch(op)
		{
			case Operation::PushLiteral:
			case Operation::ReadLocal:
			case Operation::ReadStatic:
			case Operation::NewObject:
			case Operation::Call:
			case Operation::Ret:
				return true;

			case Operation::Jump:
			case Operation::Cond:
			case Operation::WriteLocal:
			case Operation::WriteStatic:
				return false;

			case Operation::ReadField:
			case Operation::Unary:
			case Operation::Binary:
				return true;

			case Operation::WriteField:
			default:
				return false;
		}
	}

	inline Instruction() = default;
	inline Instruction(const Instruction&) = default;
	constexpr inline Instruction(Operation op, Argument arg): op(op), arg(arg) {}

	static constexpr inline Instruction literal(Storage::Value v) {
		return {Operation::PushLiteral, v};
	}

	static constexpr inline Instruction readLocal(uint32_t offset) {
		return {Operation::ReadLocal, offset};
	}

	static constexpr inline Instruction readStatic(uint32_t offset) {
		return {Operation::ReadStatic, offset};
	}

	static constexpr inline Instruction newObject(const Type* type) {
		return {Operation::NewObject, type};
	}

	static constexpr inline Instruction call(uint32_t count) {
		return {Operation::Call, count};
	}

	static constexpr inline Instruction ret(uint32_t count) {
		return {Operation::Ret, count};
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

	static constexpr inline Instruction readField(uint32_t offset) {
		return {Operation::ReadField, offset};
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
