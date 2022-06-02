#ifndef MODEL_PROGRAM_H_
#define MODEL_PROGRAM_H_

#include <vector>

#include <cstdint>

struct Program
{
	enum class OperationGroup
	{
		Binary,
		LoadStore,
		Conditional,
		Call,
		Jump,
		Immediate,
		Duplicate
	};

	enum class BinaryOperation
	{
		Add, Sub, Mul, Div, Mod,
		And, Ior, Xor, Lsh, Rsh
	};

	enum class LoadStoreOperation
	{
		LoadArgument, StoreArgument, LoadLocal, StoreLocal
	};

	enum class BranchCondition
	{
		IfEqual, IfGreater, IfLess, IfNotEqual, IfNotGreater, IfNotLess
	};

	struct Instruction
	{
		OperationGroup group;

		union
		{
			BinaryOperation binOp;

			struct
			{
				LoadStoreOperation lsOp;
				uint32_t varIdx;
			};

			struct
			{
				BranchCondition cond;
				uint32_t targetBlockIdx;
			};

			uint32_t value;
		};

	private:
		template<BinaryOperation op>
		static inline constexpr auto binary()
		{
			Instruction ret{OperationGroup::Binary};
			ret.binOp = op;
			return ret;
		}

		template<LoadStoreOperation op>
		static inline constexpr auto loadStore(uint32_t varIdx)
		{
			Instruction ret{OperationGroup::LoadStore};
			ret.lsOp = op;
			ret.varIdx = varIdx;
			return ret;
		}

		template<BranchCondition cond>
		static inline constexpr auto conditional(uint32_t target)
		{
			Instruction ret{OperationGroup::Conditional};
			ret.cond = cond;
			ret.targetBlockIdx = target;
			return ret;
		}

	public:
		static inline constexpr auto add() { return binary<BinaryOperation::Add>(); }
		static inline constexpr auto sub() { return binary<BinaryOperation::Sub>(); }
		static inline constexpr auto mul() { return binary<BinaryOperation::Mul>(); }
		static inline constexpr auto div() { return binary<BinaryOperation::Div>(); }
		static inline constexpr auto mod() { return binary<BinaryOperation::Mod>(); }
		static inline constexpr auto aAnd() { return binary<BinaryOperation::And>(); }
		static inline constexpr auto aOr() { return binary<BinaryOperation::Ior>(); }
		static inline constexpr auto aXor() { return binary<BinaryOperation::Xor>(); }
		static inline constexpr auto lsh() { return binary<BinaryOperation::Lsh>(); }
		static inline constexpr auto rsh() { return binary<BinaryOperation::Rsh>(); }

		static inline constexpr auto loadLocal(uint32_t varIdx) { return loadStore<LoadStoreOperation::LoadLocal>(varIdx); }
		static inline constexpr auto loadArgument(uint32_t varIdx) { return loadStore<LoadStoreOperation::LoadArgument>(varIdx); }
		static inline constexpr auto storeLocal(uint32_t varIdx) { return loadStore<LoadStoreOperation::StoreLocal>(varIdx); }
		static inline constexpr auto storeArgument(uint32_t varIdx) { return loadStore<LoadStoreOperation::StoreArgument>(varIdx); }

		static inline constexpr auto jeq(uint32_t target) { return conditional<BranchCondition::IfEqual>(target); }
		static inline constexpr auto jne(uint32_t target) { return conditional<BranchCondition::IfNotEqual>(target); }
		static inline constexpr auto jgt(uint32_t target) { return conditional<BranchCondition::IfGreater>(target); }
		static inline constexpr auto jle(uint32_t target) { return conditional<BranchCondition::IfNotGreater>(target); }
		static inline constexpr auto jlt(uint32_t target) { return conditional<BranchCondition::IfLess>(target); }
		static inline constexpr auto jge(uint32_t target) { return conditional<BranchCondition::IfNotLess>(target); }

		static inline constexpr auto jmp(uint32_t target)
		{
			Instruction ret{OperationGroup::Jump};
			ret.targetBlockIdx = target;
			return ret;
		}

		static inline constexpr auto call()
		{
			return Instruction {OperationGroup::Call};
		}

		static inline constexpr auto imm(uint32_t v)
		{
			Instruction ret{OperationGroup::Immediate};
			ret.value = v;
			return ret;
		}

		static inline constexpr auto dup()
		{
			return Instruction{OperationGroup::Duplicate};
		}
	};

	using Block = std::vector<Instruction>;
	using Body = std::vector<Block>;

	struct FrameInfo
	{
		uint32_t maxStack, nLocals, nArgs;
	};

	struct Function
	{
		FrameInfo info;
		Body body;
	};

	std::vector<Function> functions;

	std::vector<uint32_t> interpret(const std::vector<uint32_t> &args, size_t stackSize = 4096) const;
};

#endif /* MODEL_PROGRAM_H_ */

