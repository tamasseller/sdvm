#ifndef COMPILER_RVALUE_H_
#define COMPILER_RVALUE_H_

#include "ValueType.h"
#include "Line.h"

#include "assert.h"

#include <functional>
#include <vector>

namespace comp {

struct RValue
{
	struct Data {
		std::function<void(std::vector<Line>&)> generate;
		ValueType type;
		inline Data(decltype(generate) generate, ValueType type): generate(generate), type(type) {}
	};

	std::shared_ptr<Data> data;

	inline RValue(decltype(Data::generate) generate, ValueType type):
			data(std::make_shared<Data>(generate, type)) {}

	inline RValue(RValue a, RValue b, prog::Instruction::BinaryOpType op, PrimitiveType result):
			data(std::make_shared<Data>([a, b, op](std::vector<Line>& cw) {
				b.data->generate(cw);
				a.data->generate(cw);
				cw.push_back(Line(op));
			},
			ValueType::primitive(result))) {}

	template<prog::Instruction::BinaryOpType ifInt, PrimitiveType intResult, prog::Instruction::BinaryOpType ifFloat, PrimitiveType floatResult>
	inline RValue numericIn(RValue o)
	{
		assert(data->type.kind == TypeKind::Value);
		assert(o.data->type.kind == TypeKind::Value);

		if(data->type.primitiveType == PrimitiveType::Integer)
		{
			assert(o.data->type.primitiveType == PrimitiveType::Integer);
			return {*this, o, ifInt, intResult};
		}
		else
		{
			assert(data->type.primitiveType == PrimitiveType::Floating);
			assert(o.data->type.primitiveType == PrimitiveType::Floating);
			return {*this, o, ifFloat, floatResult};
		}
	}

	template<prog::Instruction::BinaryOpType ifInt, prog::Instruction::BinaryOpType ifFloat>
	inline RValue numericInNumericOut(const RValue& o) {
		return numericIn<ifInt, PrimitiveType::Integer, ifFloat, PrimitiveType::Floating>(o);
	}

	inline RValue operator +(const RValue& o) {
		return numericInNumericOut<prog::Instruction::BinaryOpType::AddI, prog::Instruction::BinaryOpType::AddF>(o);
	}

	inline RValue operator -(const RValue& o) {
		return numericInNumericOut<prog::Instruction::BinaryOpType::SubI, prog::Instruction::BinaryOpType::SubF>(o);
	}

	inline RValue operator *(const RValue& o) {
		return numericInNumericOut<prog::Instruction::BinaryOpType::MulI, prog::Instruction::BinaryOpType::MulF>(o);
	}

	inline RValue operator /(const RValue& o) {
		return numericInNumericOut<prog::Instruction::BinaryOpType::DivI, prog::Instruction::BinaryOpType::DivF>(o);
	}

	template<prog::Instruction::BinaryOpType ifInt, prog::Instruction::BinaryOpType ifFloat>
	inline RValue numericInLogicalOut(const RValue& o) {
		return numericIn<ifInt, PrimitiveType::Logical, ifFloat, PrimitiveType::Logical>(o);
	}

	inline RValue operator ==(const RValue& o) {
		return numericInLogicalOut<prog::Instruction::BinaryOpType::EqI, prog::Instruction::BinaryOpType::EqF>(o);
	}

	inline RValue operator !=(const RValue& o) {
		return numericInLogicalOut<prog::Instruction::BinaryOpType::NeI, prog::Instruction::BinaryOpType::NeF>(o);
	}

	inline RValue operator <(const RValue& o) {
		return numericInLogicalOut<prog::Instruction::BinaryOpType::LtI, prog::Instruction::BinaryOpType::LtF>(o);
	}

	inline RValue operator >(const RValue& o) {
		return numericInLogicalOut<prog::Instruction::BinaryOpType::GtI, prog::Instruction::BinaryOpType::GtF>(o);
	}

	inline RValue operator <=(const RValue& o) {
		return numericInLogicalOut<prog::Instruction::BinaryOpType::LeI, prog::Instruction::BinaryOpType::LeF>(o);
	}

	inline RValue operator >=(const RValue& o) {
		return numericInLogicalOut<prog::Instruction::BinaryOpType::GeI, prog::Instruction::BinaryOpType::GeF>(o);
	}

	template<prog::Instruction::BinaryOpType op, PrimitiveType input, PrimitiveType result>
	inline RValue specificIn(const RValue& o)
	{
		assert(data->type.kind == TypeKind::Value);
		assert(o.data->type.kind == TypeKind::Value);
		assert(data->type.primitiveType == input);
		assert(o.data->type.primitiveType == input);
		return {*this, o, op, result};
	}

	inline RValue operator %(const RValue& o) {
		return specificIn<prog::Instruction::BinaryOpType::Mod, PrimitiveType::Integer, PrimitiveType::Integer>(o);
	}

	inline RValue operator <<(const RValue& o) {
		return specificIn<prog::Instruction::BinaryOpType::ShlI, PrimitiveType::Integer, PrimitiveType::Integer>(o);
	}

	inline RValue operator >>(const RValue& o) {
		return specificIn<prog::Instruction::BinaryOpType::ShrU, PrimitiveType::Integer, PrimitiveType::Integer>(o);
	}

	inline RValue operator &(const RValue& o) {
		return specificIn<prog::Instruction::BinaryOpType::AndI, PrimitiveType::Integer, PrimitiveType::Integer>(o);
	}

	inline RValue operator |(const RValue& o) {
		return specificIn<prog::Instruction::BinaryOpType::OrI, PrimitiveType::Integer, PrimitiveType::Integer>(o);
	}

	inline RValue operator ^(const RValue& o) {
		return specificIn<prog::Instruction::BinaryOpType::XorI, PrimitiveType::Integer, PrimitiveType::Integer>(o);
	}

	inline RValue operator ||(const RValue& o) {
		return specificIn<prog::Instruction::BinaryOpType::AndL, PrimitiveType::Logical, PrimitiveType::Logical>(o);
	}

	inline RValue operator &&(const RValue& o) {
		return specificIn<prog::Instruction::BinaryOpType::OrL, PrimitiveType::Logical, PrimitiveType::Integer>(o);
	}
};

} //namespace comp

#endif /* COMPILER_RVALUE_H_ */
