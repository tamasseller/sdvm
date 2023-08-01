#include "FunctionBuilder.h"
#include "CodeWriter.h"

#include "ProgramBuilder.h"

using namespace comp;

FunctionBuilder::FunctionBuilder(ProgramBuilder& pb, std::optional<ValueType> ret, std::vector<ValueType> args):
		pb(pb), retType(ret), frameBuilder(pb.type({}, true))
{
	frameBuilder->addField(ValueType::dynamic());  // previous frame
	frameBuilder->addField(ValueType::integer());  // tosOffset
	std::for_each(args.cbegin(), args.cend(), [this](const auto& vt){return this->frameBuilder->addField(vt);});
	argTypes = args;
}

prog::Function FunctionBuilder::operator()()
{
	CodeWriter cw;

	for(const auto &c: code) {
		c(cw);
	}

	const auto opstackSize = cw.maxStackDepth + (cw.hasCall ? prog::Frame::callerStackExtra : 0);
	const auto opstackOffset = frameBuilder->size();

	for(int i = 0; i < opstackSize; i++)
	{
		frameBuilder->addField(ValueType::native());
	}

	return prog::Function
	{
		.frame = prog::Frame {
			.opStackOffset = opstackOffset,
			.frameTypeIndex = frameBuilder.idx
		},
		.code = std::move(cw.code)
	};
}

RValue FunctionBuilder::arg(size_t n)
{
	assert(n < argTypes.size());

	return RValue
	{
		.type = argTypes[n],
		.manifest = [this, n](CodeWriter& cw) {
			cw.write(prog::Instruction::readLocal(n));
		}
	};
}

void FunctionBuilder::setLocal(const ClassBuilder::FieldHandle &l, const RValue& b)
{
	// TODO check assignability

	code.push_back([this, l, b](CodeWriter& cw){
		b.manifest(cw);
		cw.write(prog::Instruction::writeLocal(pb.getFieldOffset(l)));
	});
}

void FunctionBuilder::ret(const RValue& v)
{
	code.push_back([v](CodeWriter& cw){
		v.manifest(cw);
		cw.write(prog::Instruction::retVal());
	});
}

void FunctionBuilder::ret() {
	code.push_back([](CodeWriter& cw){
		cw.write(prog::Instruction::ret());
	});
}

RValue FunctionBuilder::addi(const RValue& a, const RValue& b)
{
	assert(a.type.kind == TypeKind::Value);
	assert(a.type.primitiveType == PrimitiveType::Integer);
	assert(b.type.kind == TypeKind::Value);
	assert(b.type.primitiveType == PrimitiveType::Integer);

	return RValue
	{
		.type = ValueType::integer(),
		.manifest = [a, b](CodeWriter& cw){
			b.manifest(cw);
			a.manifest(cw);
			cw.write(prog::Instruction::binary(prog::Instruction::BinaryOpType::AddI));
		}
	};
}

RValue FunctionBuilder::create(Handle<ClassBuilder> t)
{
	return RValue {
		.type = ValueType::reference(t.idx),
		.manifest = [t](CodeWriter& cw){
			cw.write(prog::Instruction::newObject(t.idx));
		}
	};
}
