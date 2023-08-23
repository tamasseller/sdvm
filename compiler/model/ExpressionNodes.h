#ifndef COMPILER_MODEL_EXPRESSIONNODES_H_
#define COMPILER_MODEL_EXPRESSIONNODES_H_

#include "Value.h"

#include "Field.h"
#include "Function.h"

namespace comp {

template<class Child, class Base = RValue>
struct ValueBase: Base
{
	inline virtual void accept(const ValueVisitor& v) const override final {
		v.visit(*static_cast<const Child*>(this));
	}

	inline virtual ~ValueBase() = default;
};

template<class Child>
struct LValueBase: ValueBase<Child, LValue> {};

struct Local: LValueBase<Local>
{
	const ValueType type;

	inline Local(ValueType type): type(type) {}

	inline virtual ValueType getType() override { return type; }
};

struct Argument: LValueBase<Argument>
{
	const ValueType type;
	const size_t idx;

	inline Argument(ValueType type, size_t idx): type(type), idx(idx) {}

	inline virtual ValueType getType() override { return type; }
};

struct Global: LValueBase<Global>
{
	const StaticField field;

	inline Global(const StaticField& field): field(field) {}
	inline virtual ~Global() = default;

	inline virtual ValueType getType() override { return field.getType(); }
};

struct Dereference: LValueBase<Dereference>
{
	const std::shared_ptr<RValue> object;
	const Field field;

	inline Dereference(std::shared_ptr<RValue> object, const Field& field): object(object), field(field) {}
	inline virtual ~Dereference() = default;

	inline virtual ValueType getType() override { return field.getType(); }
};

struct Literal: ValueBase<Literal>
{
	const ValueType type;

	union
	{
		const int integer;
		const float floating;
		const bool logical;
	};

	inline Literal(int integer): type(ValueType::integer()), integer(integer) {}
	inline Literal(float floating): type(ValueType::floating()), floating(floating) {}
	inline Literal(bool logical): type(ValueType::logical()), logical(logical) {}

	inline virtual ValueType getType() override { return type; }
};

struct Unary: ValueBase<Unary>
{
	enum class Operation
	{
		Neg, I2F, F2I, Not
	};

	const Operation op;
	const std::shared_ptr<RValue> arg;

	Unary(Operation op, std::shared_ptr<RValue> arg): op(op), arg(arg) {}

	inline virtual ValueType getType() override
	{
		switch(op)
		{
			case Operation::Neg:
			case Operation::F2I:
				return ValueType::integer();
			case Operation::I2F:
				return ValueType::floating();
			case Operation::Not:
				return ValueType::logical();
		}

		return ValueType::native();
	}
};

struct Binary: ValueBase<Binary>
{
	enum class Operation
	{
		AddI,
		MulI,
		SubI,
		DivI,
		Mod,
		ShlI,
		ShrI,
		ShrU,
		AndI,
		OrI,
		XorI,
		AddF,
		MulF,
		SubF,
		DivF,
		Eq,
		Ne,
		LtI,
		GtI,
		LeI,
		GeI,
		LtU,
		GtU,
		LeU,
		GeU,
		LtF,
		GtF,
		LeF,
		GeF,
		And,
		Or
	};

	const Operation op;
	const std::shared_ptr<RValue> first, second;

	Binary(Operation op, std::shared_ptr<RValue> first, std::shared_ptr<RValue> second): op(op), first(first), second(second) {}

	inline virtual ValueType getType() override
	{
		switch(op)
		{
			case Operation::AddI:
			case Operation::MulI:
			case Operation::SubI:
			case Operation::DivI:
			case Operation::Mod:
			case Operation::ShlI:
			case Operation::ShrI:
			case Operation::ShrU:
			case Operation::AndI:
			case Operation::OrI:
			case Operation::XorI:
				return ValueType::integer();
			case Operation::AddF:
			case Operation::MulF:
			case Operation::SubF:
			case Operation::DivF:
				return ValueType::floating();
			case Operation::Eq:
			case Operation::Ne:
			case Operation::LtI:
			case Operation::GtI:
			case Operation::LeI:
			case Operation::GeI:
			case Operation::LtU:
			case Operation::GtU:
			case Operation::LeU:
			case Operation::GeU:
			case Operation::LtF:
			case Operation::GtF:
			case Operation::LeF:
			case Operation::GeF:
			case Operation::And:
			case Operation::Or:
				return ValueType::logical();
		}

		return ValueType::native();
	}
};

struct Ternary: ValueBase<Ternary>
{
	const std::shared_ptr<RValue> condition, then, otherwise;

	Ternary(std::shared_ptr<RValue> condition, std::shared_ptr<RValue> then, std::shared_ptr<RValue> otherwise):
		condition(condition), then(then), otherwise(otherwise) {}

	inline virtual ValueType getType() override {
		// TODO assert(then->getType() == otherwise->getType());
		return then->getType();
	}
};

struct Create: ValueBase<Create>
{
	const std::shared_ptr<Class> type;

	inline Create(decltype(type) type): type(type) {}

	inline virtual ValueType getType() override { return ValueType::reference(type); }
};

struct Call: ValueBase<Call>
{
	std::shared_ptr<Function> fn;
	std::vector<std::shared_ptr<RValue>> args;

	inline Call(std::shared_ptr<Function> fn, std::vector<std::shared_ptr<RValue>> args): fn(fn), args(args) {}

	inline virtual ValueType getType() override
	{
		assert(fn->ret.size() == 1); // compile error

		return fn->ret[0];
	}
};

}  // namespace comp

#endif /* COMPILER_MODEL_EXPRESSIONNODES_H_ */
