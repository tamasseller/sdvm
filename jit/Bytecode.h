#ifndef JIT_BYTECODE_H_
#define JIT_BYTECODE_H_

#include <cstdint>

struct Bytecode
{
	struct FunctionInfo
	{
		uint32_t nLabels, nRet;
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
			Add, Sub, Mul, Div, Mod, And, Ior, Xor, Lsh, Rsh, Ash
		};

		enum class Condition
		{
			Equal, NotEqual,
			UnsignedGreater, UnsignedNotGreater,
			UnsignedLess, UnsignedNotLess,
			SignedGreater, SignedNotGreater,
			SignedLess, SignedNotLess
		};

		OperationGroup g;

		enum class DupDirection
		{
			In, Out
		};

		enum class DupTarget
		{
			Arg, Stack
		};

		union
		{
			struct Immediate
			{
				uint32_t value;
			} imm;

			struct Binary
			{
				BinaryOperation op;
			} bin;

			struct Conditional
			{
				Condition cond;
				uint32_t targetIdx;
			} cond;

			struct Jump
			{
				uint32_t targetIdx;
			} jump;

			struct Label
			{
				uint32_t stackAdjustment;
			} label;

			struct Move
			{
				DupDirection dir;
				DupTarget target;
				uint32_t idx;
			} move;

			struct Call
			{
				uint32_t nArgs, nRet;
			} call;
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

	static inline constexpr auto move(Instruction::DupDirection dir, Instruction::DupTarget target, uint32_t idx)
	{
		auto ret = Instruction{.g = Instruction::OperationGroup::Move};
		ret.label = Instruction::Move{.dir = dir, .target = target, .idx = idx};
		return ret;
	}

	static inline constexpr auto ldArg(uint32_t idx) { return move(Instruction::DupDirection::In,  Instruction::DupTarget::Arg, idx); }
	static inline constexpr auto ldLoc(uint32_t idx) { return move(Instruction::DupDirection::In,  Instruction::DupTarget::Stack, idx); }
	static inline constexpr auto stArg(uint32_t idx) { return move(Instruction::DupDirection::Out, Instruction::DupTarget::Arg, idx); }
	static inline constexpr auto stLoc(uint32_t idx) { return move(Instruction::DupDirection::Out, Instruction::DupTarget::Stack, idx); }

	static inline constexpr auto call(uint32_t nArgs, uint32_t nRet)
	{
		auto ret = Instruction{.g = Instruction::OperationGroup::Call};
		ret.label = Instruction::Call{.nArgs = nArgs, .nRet = nRet};
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
