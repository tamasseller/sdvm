#ifndef JIT_BYTECODE_H_
#define JIT_BYTECODE_H_

#include <cstdint>

struct Bytecode
{
	struct FunctionInfo
	{
		uint32_t nArgs, nRet;
	};

	struct Instruction
	{
		enum class OperationGroup: uint8_t
		{
			Immediate,
			Binary,
			Conditional,
			Jump,
			Label,
			Move,
			Call,
			Return,
			Invalid
		};

		enum class BinaryOperation: uint8_t
		{
			Add, Sub, Lsh, Rsh, Ash, And, Ior, Xor, Mul, Div, Mod
		};

		enum class Condition: uint8_t
		{
			Equal, NotEqual,
			UnsignedGreater, UnsignedNotGreater,
			UnsignedLess, UnsignedNotLess,
			SignedGreater, SignedNotGreater,
			SignedLess, SignedNotLess
		};

		enum class MoveOperation: uint8_t
		{
			Pull, Shove, Drop
		};

		OperationGroup g;

		union
		{
			Condition cond;
			MoveOperation moveOp;
			BinaryOperation binOp;
			uint8_t rawSpecifier;
		};

		union
		{
			uint32_t targetIdx;
			uint32_t immValue;
			uint32_t param;

			struct
			{
				uint16_t nArgs, nRet;
			};

			uint32_t rawParameter;
		};

		constexpr inline operator uint64_t() const
		{
			return ((uint64_t)(uint8_t)g << 56) | ((uint64_t)rawSpecifier << 48) | rawParameter;
		}

		constexpr inline Instruction(): Instruction(0ull) {}
		constexpr inline Instruction(const Instruction&) = default;
		constexpr inline Instruction(uint64_t b): g((OperationGroup)(b >> 56)), rawSpecifier(b >> 48), rawParameter(b) {}
	};

	static_assert(sizeof(Instruction) == 8);

	static inline constexpr auto immediate(uint32_t value)
	{
		Instruction ret;
		ret.g = Instruction::OperationGroup::Immediate;
		ret.immValue = value;
		return ret;
	}

	static inline constexpr auto binary(Instruction::BinaryOperation op)
	{
		Instruction ret;
		ret.g = Instruction::OperationGroup::Binary;
		ret.binOp = op;
		return ret;
	}

	static inline constexpr auto add()  { return binary(Instruction::BinaryOperation::Add); }
	static inline constexpr auto sub()  { return binary(Instruction::BinaryOperation::Sub); }
	static inline constexpr auto mul()  { return binary(Instruction::BinaryOperation::Mul); }
	static inline constexpr auto div()  { return binary(Instruction::BinaryOperation::Div); }
	static inline constexpr auto mod()  { return binary(Instruction::BinaryOperation::Mod); }
	static inline constexpr auto aAnd() { return binary(Instruction::BinaryOperation::And); }
	static inline constexpr auto aOr()  { return binary(Instruction::BinaryOperation::Ior); }
	static inline constexpr auto aXor() { return binary(Instruction::BinaryOperation::Xor); }
	static inline constexpr auto lsh()  { return binary(Instruction::BinaryOperation::Lsh); }
	static inline constexpr auto rsh()  { return binary(Instruction::BinaryOperation::Rsh); }
	static inline constexpr auto ash()  { return binary(Instruction::BinaryOperation::Ash); }

	static inline constexpr auto conditional(Instruction::Condition cond, uint32_t targetIdx)
	{
		Instruction ret;
		ret.g = Instruction::OperationGroup::Conditional;
		ret.cond = cond;
		ret.targetIdx = targetIdx;
		return ret;
	}

	static inline constexpr auto jeq(uint32_t targetIdx)  { return conditional(Instruction::Condition::Equal, targetIdx); }
	static inline constexpr auto jne(uint32_t targetIdx)  { return conditional(Instruction::Condition::Equal, targetIdx); }
	static inline constexpr auto jult(uint32_t targetIdx) { return conditional(Instruction::Condition::UnsignedLess, targetIdx); }
	static inline constexpr auto jugt(uint32_t targetIdx) { return conditional(Instruction::Condition::UnsignedGreater, targetIdx); }
	static inline constexpr auto juge(uint32_t targetIdx) { return conditional(Instruction::Condition::UnsignedNotLess, targetIdx); }
	static inline constexpr auto jule(uint32_t targetIdx) { return conditional(Instruction::Condition::UnsignedNotGreater, targetIdx); }
	static inline constexpr auto jslt(uint32_t targetIdx) { return conditional(Instruction::Condition::SignedLess, targetIdx); }
	static inline constexpr auto jsgt(uint32_t targetIdx) { return conditional(Instruction::Condition::SignedGreater, targetIdx); }
	static inline constexpr auto jsge(uint32_t targetIdx) { return conditional(Instruction::Condition::SignedNotLess, targetIdx); }
	static inline constexpr auto jsle(uint32_t targetIdx) { return conditional(Instruction::Condition::SignedNotGreater, targetIdx); }

	static inline constexpr auto jump(uint32_t targetIdx)
	{
		Instruction ret;
		ret.g = Instruction::OperationGroup::Jump;
		ret.targetIdx = targetIdx;
		return ret;
	}

	static inline constexpr auto label()
	{
		Instruction ret;
		ret.g = Instruction::OperationGroup::Label;
		return ret;
	}

	static inline constexpr auto move(Instruction::MoveOperation op, uint32_t idx)
	{
		Instruction ret;
		ret.g = Instruction::OperationGroup::Move;
		ret.moveOp = op;
		ret.param = idx;
		return ret;
	}

	static inline constexpr auto pull(uint32_t idx) { return move(Instruction::MoveOperation::Pull, idx); }
	static inline constexpr auto shove(uint32_t idx) { return move(Instruction::MoveOperation::Shove, idx); }
	static inline constexpr auto drop(uint32_t count) { return move(Instruction::MoveOperation::Drop, count); }

	static inline constexpr auto call(uint32_t nArgs, uint32_t nRet)
	{
		Instruction ret;
		ret.g = Instruction::OperationGroup::Call;
		ret.nArgs = nArgs;
		ret.nRet = nRet;
		return ret;
	}

	static inline constexpr auto ret() {
		Instruction ret;
		ret.g = Instruction::OperationGroup::Return;
		return ret;
	}

	class FunctionReader
	{
		uint64_t (*doRead)(FunctionReader*);
		FunctionInfo (*doReset)(FunctionReader*);

	public:
		inline FunctionReader(decltype(doRead) doRead, decltype(doReset) doReset):
			doRead(doRead), doReset(doReset) {}

		inline bool read(Instruction &isn)
		{
			isn = doRead(this);
			return isn.g != Bytecode::Instruction::OperationGroup::Invalid;
		}

		inline FunctionInfo reset()
		{
			return doReset(this);
		}
	};
};

#endif /* JIT_BYTECODE_H_ */
