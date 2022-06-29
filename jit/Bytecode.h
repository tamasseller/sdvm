#ifndef JIT_BYTECODE_H_
#define JIT_BYTECODE_H_

#include <cstdint>

struct Bytecode
{
	struct FunctionInfo
	{
		uint32_t nLabels, nArgs, nRet;
		bool hasNonTailCall;
	};

	struct Instruction
	{
		enum class OperationGroup
		{
			Immediate,
			Binary,
			Conditional,
			Jump,
			Label,
			Move,
			Call,
			Return
		};

		enum class BinaryOperation
		{
			Add, Sub, And, Ior, Xor, Lsh, Rsh, Ash, Mul, Div, Mod
		};

		enum class Condition
		{
			Equal, NotEqual,
			UnsignedGreater, UnsignedNotGreater,
			UnsignedLess, UnsignedNotLess,
			SignedGreater, SignedNotGreater,
			SignedLess, SignedNotLess
		};

		enum class MoveOperation
		{
			Pull, Shove, Drop
		};

		struct Immediate
		{
			uint32_t value;
		};

		struct Binary
		{
			BinaryOperation op;
		};

		struct Conditional
		{
			Condition cond;
			uint32_t targetIdx;
		};

		struct Jump
		{
			uint32_t targetIdx;
		};

		struct Label
		{
			uint32_t stackAdjustment;
		};

		struct Move
		{
			MoveOperation op;
			uint32_t param;
		};

		struct Call
		{
			uint32_t nArgs, nRet;
		};

		OperationGroup g;

		union
		{
			Immediate imm;
			Binary bin;
			Conditional cond;
			Jump jump;
			Label label;
			Move move;
			Call call;
		};
	};

	static inline constexpr auto immediate(uint32_t value)
	{
		auto ret = Instruction{.g = Instruction::OperationGroup::Immediate};
		ret.imm.value = value;
		return ret;
	}

	static inline constexpr auto binary(Instruction::BinaryOperation op)
	{
		auto ret = Instruction{.g = Instruction::OperationGroup::Binary};
		ret.bin.op = op;
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
		auto ret = Instruction{.g = Instruction::OperationGroup::Conditional};
		ret.cond = Instruction::Conditional{.cond = cond, .targetIdx = targetIdx};
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
		auto ret = Instruction{.g = Instruction::OperationGroup::Jump};
		ret.jump = Instruction::Jump{.targetIdx = targetIdx};
		return ret;
	}

	static inline constexpr auto label(uint32_t stackAdjustment = 0)
	{
		auto ret = Instruction{.g = Instruction::OperationGroup::Label};
		ret.label = Instruction::Label{.stackAdjustment = stackAdjustment};
		return ret;
	}

	static inline constexpr auto move(Instruction::MoveOperation op, uint32_t idx)
	{
		auto ret = Instruction{.g = Instruction::OperationGroup::Move};
		ret.move = Instruction::Move{.op = op, .param = idx};
		return ret;
	}

	static inline constexpr auto pull(uint32_t idx) { return move(Instruction::MoveOperation::Pull, idx); }
	static inline constexpr auto shove(uint32_t idx) { return move(Instruction::MoveOperation::Shove, idx); }
	static inline constexpr auto drop(uint32_t count) { return move(Instruction::MoveOperation::Drop, count); }

	static inline constexpr auto call(uint32_t nArgs, uint32_t nRet)
	{
		auto ret = Instruction{.g = Instruction::OperationGroup::Call};
		ret.call = Instruction::Call{.nArgs = nArgs, .nRet = nRet};
		return ret;
	}

	static inline constexpr auto ret() {
		return Instruction{.g = Instruction::OperationGroup::Return};
	}

	class InstructionStreamReader
	{
		bool (*doRead)(InstructionStreamReader*, Instruction&);

	public:
		inline InstructionStreamReader(decltype(doRead) doRead): doRead(doRead) {}
		inline bool operator()(Instruction &isn) { return doRead(this, isn); }
	};
};

#endif /* JIT_BYTECODE_H_ */
