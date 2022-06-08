#ifndef MODEL_PROGRAM_H_
#define MODEL_PROGRAM_H_

#include <cstdint>

#define X_BINARY_OPERATOR_LIST() 	\
	X(Add, +)						\
	X(Sub, -)						\
	X(Mul, *)                       \
	X(Div, /)						\
	X(Mod, %)                       \
	X(And, &)                       \
	X(Ior, |)                       \
	X(Xor, ^)						\
	X(Lsh, <<)                      \
	X(Rsh, >>)                      \
	X(Equal, ==)                    \
	X(Greater, >)                   \
	X(Less, <)                      \
	X(NotEqual, !=)                 \
	X(NotGreater, <=)               \
	X(NotLess, >=)					\
	X(Both, &&)						\
	X(Either, ||)

struct Visa
{
	enum class OperationGroup
	{
		Binary,
		Load,
		Store,
		Conditional,
		Jump,
		Call,
		Return,
		Immediate,
		Duplicate
	};

	enum class BinaryOperation
	{
#define X(sym, op) sym,
		X_BINARY_OPERATOR_LIST()
#undef X
	};

	enum class LoadStoreDestination
	{
		Argument, Local
	};

	struct Instruction
	{
		OperationGroup group;

		union
		{
			BinaryOperation binOp;

			struct
			{
				LoadStoreDestination dest;
				uint32_t varIdx;
			};

			uint32_t targetBlockIdx;

			uint32_t value;
		};

	private:
		template<bool load, LoadStoreDestination dst>
		static inline constexpr auto loadStore(uint32_t varIdx)
		{
			Instruction ret{load ? OperationGroup::Load : OperationGroup::Store};
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

		static inline constexpr auto loadLocal(uint32_t varIdx) { return loadStore<true, LoadStoreDestination::Local>(varIdx); }
		static inline constexpr auto loadArgument(uint32_t varIdx) { return loadStore<true, LoadStoreDestination::Argument>(varIdx); }
		static inline constexpr auto storeLocal(uint32_t varIdx) { return loadStore<false, LoadStoreDestination::Local>(varIdx); }
		static inline constexpr auto storeArgument(uint32_t varIdx) { return loadStore<false, LoadStoreDestination::Argument>(varIdx); }

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

	static inline constexpr bool stackBalance(OperationGroup g, uint32_t &ret)
	{
		switch(g)
		{
		case OperationGroup::Binary: 		ret = -1; return true;
		case OperationGroup::Load: 			ret = 1;  return true;
		case OperationGroup::Store: 		ret = -1; return true;
		case OperationGroup::Conditional: 	ret = -1; return true;
		case OperationGroup::Jump: 			ret = 0;  return true;
		case OperationGroup::Immediate: 	ret = 1;  return true;
		case OperationGroup::Duplicate: 	ret = 1;  return true;
		default: return false;
		}
	}
};

#endif /* MODEL_PROGRAM_H_ */
