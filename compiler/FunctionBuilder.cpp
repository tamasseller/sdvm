#include "FunctionBuilder.h"
#include "ProgramBuilder.h"
#include "FrameType.h"

using namespace comp;

void FunctionBuilder::write(prog::Instruction isn)
{
	hasCall = hasCall || isn.op == prog::Instruction::Operation::Call || isn.op == prog::Instruction::Operation::CallV;
	stackDepth += isn.stackBalance();
	assert(0 <= stackDepth);
	maxStackDepth = std::max(maxStackDepth, stackDepth);
	code.push_back(isn);
}

FunctionBuilder::FunctionBuilder(std::optional<Type> ret, std::vector<Type> args):
		retType(ret), args(args), nextLocal(args.size())
{
	for(auto i = 0u; i < args.size(); i++) {
		if(args[i].kind == TypeKind::Reference)
		{
			frameRefIndices.push_back(i);
		}
	}
}

prog::Function FunctionBuilder::operator()(std::vector<obj::Type>& types)
{
	const auto frameTypeIndex = types.size();
	types.push_back(makeFrameType(nextLocal, std::move(frameRefIndices), maxStackDepth + (hasCall ? prog::Frame::callerStackExtra : 0)));

	prog::Function ret =
	{
		.frame = prog::Frame {
			.opStackOffset = prog::Frame::offsetToLocals + nextLocal,
			.frameTypeIndex = frameTypeIndex
		},
		.code = std::move(code)
	};

	return ret;
}

RValue FunctionBuilder::arg(size_t n) {
	assert(n < args.size());
	return RValue {
		.type = args[n],
		.manifest = [this, n](){write(prog::Instruction::readValueLocal(n));}
	};
}

void FunctionBuilder::ret(const RValue& v)
{
	v.manifest();
	write(v.type.kind == TypeKind::Reference ? prog::Instruction::retRef() : prog::Instruction::retVal());
}

void FunctionBuilder::ret() {
	write(prog::Instruction::ret());
}

RValue FunctionBuilder::addi(const RValue& a, const RValue& b)
{
	assert(a.type.kind == TypeKind::Value);
	assert(a.type.primitiveType == ValueType::Integer);
	assert(b.type.kind == TypeKind::Value);
	assert(b.type.primitiveType == ValueType::Integer);

	return RValue {
		.type = Type::integer(),
		.manifest = [this, a, b](){
			b.manifest();
			a.manifest();
			write(prog::Instruction::binary(prog::Instruction::BinaryOpType::AddI));
		}
	};
}
