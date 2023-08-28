#ifndef COMPILER_IR_OPERATION_H_
#define COMPILER_IR_OPERATION_H_

#include "Temporary.h"

#include "meta/Operation.h"

#include "compiler/ast/Class.h"
#include "compiler/ast/Field.h"
#include "compiler/ast/Function.h"

/*
 * A proto-instruction
 */
namespace comp {
namespace ir {

struct Literal: OperationBase<Literal>
{
	const std::shared_ptr<Temporary> target;
	const int integer;

	inline Literal(decltype(target) target, int integer): target(target), integer(integer) {}
};

struct Unary: OperationBase<Unary>
{
	enum class Op
	{
		Neg, I2F, F2I, Not
	};

	const std::shared_ptr<Temporary> target, source;
	const Op op;

	inline Unary(decltype(target) target, decltype(source) source, decltype(op) op):
		target(target), source(source), op(op) {}
};

struct Create: OperationBase<Create>
{
	const std::shared_ptr<Temporary> target;
	const std::shared_ptr<ast::Class> type;

	inline Create(decltype(target) target, decltype(type) type): target(target), type(type) {}
};

struct LoadField: OperationBase<LoadField>
{
	const std::shared_ptr<Temporary> target, object;
	const ast::Field field;

	inline LoadField(decltype(target) target, decltype(object) object, decltype(field) field):
		target(target), object(object), field(field) {}
};

struct StoreField: OperationBase<StoreField>
{
	const std::shared_ptr<Temporary> source, object;
	const ast::Field field;

	inline StoreField(decltype(source) source, decltype(object) object, decltype(field) field):
		source(source), object(object), field(field) {}
};

struct LoadGlobal: OperationBase<LoadGlobal>
{
	const std::shared_ptr<Temporary> target;
	const ast::Field field;

	inline LoadGlobal(decltype(target) target, decltype(field) field): target(target), field(field) {}
};

struct StoreGlobal: OperationBase<StoreGlobal>
{
	const std::shared_ptr<Temporary> source;
	const ast::Field field;

	inline StoreGlobal(decltype(source) source, decltype(field) field): source(source), field(field) {}
};

struct Binary: OperationBase<Binary>
{
	enum class Op
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
		DivF
	};

	const std::shared_ptr<Temporary> target, first, second;
	const Op op;

	inline Binary(decltype(target) target, decltype(first) first, decltype(second) second, decltype(op) op):
		target(target), first(first), second(second), op(op) {}
};

struct Call: OperationBase<Call>
{
	const std::vector<std::shared_ptr<Temporary>> arg, ret;
	const std::shared_ptr<ast::Function> fn;

	inline Call(decltype(arg) arg, decltype(ret) ret, decltype(fn) fn): arg(arg), ret(ret), fn(fn) {}
};

} // namespace ir
} // namespace comp

#endif /* COMPILER_IR_OPERATION_H_ */
