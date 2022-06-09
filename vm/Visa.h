#ifndef MODEL_PROGRAM_H_
#define MODEL_PROGRAM_H_

#include <cstdint>

#define X_BINARY_OPERATOR_LIST()        \
    X(Add, +, "add")                    \
    X(Sub, -, "sub")                    \
    X(Mul, *, "mul")                    \
    X(Div, /, "div")                    \
    X(Mod, %, "mod")                    \
    X(And, &, "and")                    \
    X(Ior, |, "ior")                    \
    X(Xor, ^, "xor")                    \
    X(Lsh, <<, "lsh")                   \
    X(Rsh, >>, "rsh")                   \
    X(Equal, ==, "eq")                  \
    X(Greater, >, "gt")                 \
    X(Less, <, "lt")                    \
    X(NotEqual, !=, "neq")              \
    X(NotGreater, <=, "le")             \
    X(NotLess, >=, "gt")                \
    X(Both, &&, "land")                 \
    X(Either, ||, "lor")

struct Visa
{
	enum class OperationGroup
	{
		Binary,
		Move,
		Conditional,
		Jump,
		Call,
		Return,
		Immediate,
		Duplicate
	};

	enum class BinaryOperation
	{
#define X(sym, op, mn) sym,
		X_BINARY_OPERATOR_LIST()
#undef X
	};

	enum class MoveDetails
	{
		FromArgument, FromLocal, ToArgument, ToLocal
	};

	struct Instruction
	{
		OperationGroup group;

		union
		{
			BinaryOperation binOp;

			struct
			{
				MoveDetails dest;
				uint32_t varIdx;
			};

			uint32_t targetBlockIdx;

			uint32_t value;
		};

	private:
		template<MoveDetails dst>
		static inline constexpr auto loadStore(uint32_t varIdx)
		{
			Instruction ret{OperationGroup::Move};
			ret.dest = dst;
			ret.varIdx = varIdx;
			return ret;
		}

	public:
		template<BinaryOperation op>
		static inline constexpr auto binary()
		{
			Instruction ret{OperationGroup::Binary};
			ret.binOp = op;
			return ret;
		}

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
		static inline constexpr auto leq() { return binary<BinaryOperation::Equal>(); }
		static inline constexpr auto lne() { return binary<BinaryOperation::NotEqual>(); }
		static inline constexpr auto lgt() { return binary<BinaryOperation::Greater>(); }
		static inline constexpr auto lle() { return binary<BinaryOperation::NotGreater>(); }
		static inline constexpr auto llt() { return binary<BinaryOperation::Less>(); }
		static inline constexpr auto lge() { return binary<BinaryOperation::NotLess>(); }
		static inline constexpr auto lAnd() { return binary<BinaryOperation::Both>(); }
		static inline constexpr auto lOr() { return binary<BinaryOperation::Either>(); }

		static inline constexpr auto loadLocal(uint32_t varIdx) { return loadStore<MoveDetails::FromLocal>(varIdx); }
		static inline constexpr auto loadArgument(uint32_t varIdx) { return loadStore<MoveDetails::FromArgument>(varIdx); }
		static inline constexpr auto storeLocal(uint32_t varIdx) { return loadStore<MoveDetails::ToLocal>(varIdx); }
		static inline constexpr auto storeArgument(uint32_t varIdx) { return loadStore<MoveDetails::ToArgument>(varIdx); }

		static inline constexpr auto jcond(uint32_t target)
		{
			Instruction ret{OperationGroup::Conditional};
			ret.targetBlockIdx = target;
			return ret;
		}

		static inline constexpr auto jmp(uint32_t target)
		{
			Instruction ret{OperationGroup::Jump};
			ret.targetBlockIdx = target;
			return ret;
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

		static inline constexpr auto call()
		{
			Instruction ret{OperationGroup::Call};
			return ret;
		}

		static inline constexpr auto ret()
		{
			Instruction ret{OperationGroup::Return};
			return ret;
		}
	};

	struct FrameInfo
	{
		uint32_t maxStack, nLocals, nArgs, nRet;
	};

	static inline constexpr bool stackBalance(Instruction isn, uint32_t &ret)
	{
		switch(isn.group)
		{
		case OperationGroup::Binary: 		ret = -1; return true;
		case OperationGroup::Move: 			ret = (isn.dest == MoveDetails::FromArgument || isn.dest == MoveDetails::FromLocal) ? 1 : -1;  return true;
		case OperationGroup::Conditional: 	ret = -1; return true;
		case OperationGroup::Jump: 			ret = 0;  return true;
		case OperationGroup::Immediate: 	ret = 1;  return true;
		case OperationGroup::Duplicate: 	ret = 1;  return true;
		default: return false;
		}
	}
};

#endif /* MODEL_PROGRAM_H_ */
