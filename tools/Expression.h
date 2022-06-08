#ifndef TOOLS_EXPRESSION_H_
#define TOOLS_EXPRESSION_H_

#include "StatementStream.h"

struct Expression
{
	struct LValue
	{
		virtual operator StatementStream::Statement() const = 0;
		inline virtual ~LValue() = default;
	};

	struct RValue
	{
		virtual operator StatementStream::Statement() const = 0;
		inline virtual ~RValue() = default;
	};

	class BinaryOperator: public RValue
	{
		StatementStream::Statement writer;

	public:
		BinaryOperator(Visa::BinaryOperation op, const RValue &l, const RValue &r);
		virtual operator StatementStream::Statement() const override;
	};

	template<class Local>
	struct Locall: LValue
	{
		virtual operator StatementStream::Statement() const override {
			return [idx{static_cast<const Local*>(this)->idx}](StatementStream::InstructionSink& b){
				b.takeNonControl(Visa::Instruction::storeLocal(idx));
			};
		}
	};

	template<class Local>
	struct Localr: RValue
	{
		virtual operator StatementStream::Statement() const override {
			return [idx{static_cast<const Local*>(this)->idx}](StatementStream::InstructionSink& b){
				b.takeNonControl(Visa::Instruction::loadLocal(idx));
			};
		}
	};

	struct Local: Locall<Local>, Localr<Local>
	{
		uint32_t idx;

		inline Local(decltype(idx) idx): idx(idx) {}
	};

	template<class Argument>
	struct Argumentl: LValue
	{
		virtual operator StatementStream::Statement() const override {
			return [idx{static_cast<const Argument*>(this)->idx}](StatementStream::InstructionSink& b){
				b.takeNonControl(Visa::Instruction::storeArgument(idx));
			};
		}
	};

	template<class Argument>
	struct Argumentr: RValue
	{
		virtual operator StatementStream::Statement() const override {
			return [idx{static_cast<const Argument*>(this)->idx}](StatementStream::InstructionSink& b){
				b.takeNonControl(Visa::Instruction::loadArgument(idx));
			};
		}
	};

	struct Argument: Argumentl<Argument>, Argumentr<Argument>
	{
		uint32_t idx;

		inline Argument(decltype(idx) idx): idx(idx) {}
	};

	struct Immediate: RValue
	{
		uint32_t value;

		virtual operator StatementStream::Statement() const override {
			return [value{this->value}](StatementStream::InstructionSink& b){
				b.takeNonControl(Visa::Instruction::imm(value));
			};
		}

		inline Immediate(decltype(value) value): value(value) {}
	};
};

#define X(sym, op)																				\
static inline auto operator op(const Expression::RValue& l, const Expression::RValue& r) {		\
	return Expression::BinaryOperator(Visa::BinaryOperation:: sym, l, r);						\
}																								\
																								\
static inline auto operator op(const Expression::RValue& l, const Expression::Immediate& r) {	\
	return l op (const Expression::RValue&)r;													\
}																								\
																								\
static inline auto operator op(const Expression::Immediate& l, const Expression::RValue& r) {	\
	return (const Expression::RValue&)l op r;													\
}

X_BINARY_OPERATOR_LIST()
#undef X

#endif /* TOOLS_EXPRESSION_H_ */
