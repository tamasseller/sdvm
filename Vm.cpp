#include "Vm.h"

Vm::Vm(Storage& s, const Program &p):
	storage(s), program(p), staticObject(s.create(p.staticType)) {}

Storage::Ref Vm::createFrame(const Program::Function &f, Storage::Ref caller, std::vector<Storage::Value> &args, size_t argCount)
{
	assert(Program::Function::previousFrameOffset < f.frameType->length);
	assert(Program::Function::executionPointOffset < f.frameType->length);
	assert(argCount <= f.argOffsets.size());
	assert(argCount < f.frameType->length);

	const auto ret = storage.create(f.frameType);
	storage.write(ret, Program::Function::previousFrameOffset, caller);
	storage.write(ret, Program::Function::executionPointOffset, 0);

	for(auto it = f.argOffsets.begin(); it < f.argOffsets.begin() + argCount; it++)
	{
		storage.write(ret, *it, args.back());
		args.pop_back();
	}

	return ret;
}

std::optional<Storage::Value> Vm::run(std::vector<Storage::Value> args)
{
	assert(!program.functions.empty());

	const auto &f = program.functions.front();

	auto currentFrame = createFrame(f, staticObject, args, args.size());

	std::vector<Storage::Value> opStack;
	for(auto it = f.code.begin(); it != f.code.end();)
	{
		const auto &isn = *it++;

		Storage::Value in1out, in2;

		switch(isn.popCount())
		{
			case 2:
				assert(!opStack.empty());
				in2 = opStack.back();
				opStack.pop_back();
				[[fallthrough]];
			case 1:
				assert(!opStack.empty());
				in1out = opStack.back();
				opStack.pop_back();
				[[fallthrough]];
			case 0:
				break;
		}

		switch(isn.op)
		{
		case Instruction::Operation::PushLiteral:
			in1out = isn.arg.literal;
			break;
		case Instruction::Operation::ReadLocal:
			in1out = storage.read(currentFrame, isn.arg.fieldOffset);
			break;
		case Instruction::Operation::ReadStatic:
			in1out = storage.read(staticObject, isn.arg.fieldOffset);
			break;
		case Instruction::Operation::NewObject:
			in1out = storage.create(isn.arg.type);
			break;
		case Instruction::Operation::Call:
			// TODO
			break;
		case Instruction::Operation::Ret:
			// TODO
			break;
		case Instruction::Operation::Jump:
			it = f.code.begin() + isn.arg.jumpTarget;
			break;
		case Instruction::Operation::Cond:
			if(in1out.logical)
			{
				it = f.code.begin() + isn.arg.jumpTarget;
			}
			break;
		case Instruction::Operation::WriteLocal:
			storage.write(currentFrame, isn.arg.fieldOffset, in1out);
			break;
		case Instruction::Operation::WriteStatic:
			storage.write(staticObject, isn.arg.fieldOffset, in1out);
			break;
		case Instruction::Operation::ReadField:
			in1out = storage.read(in1out.reference, isn.arg.fieldOffset);
			break;
		case Instruction::Operation::Unary:
			switch(isn.arg.unOp)
			{
			case Instruction::UnaryOpType::Not:
				in1out.logical = !in1out.logical;
				break;
			case Instruction::UnaryOpType::BitwiseNegate:
				in1out.integer = ~in1out.integer;
				break;
			}
			break;
		case Instruction::Operation::Binary:
			switch(isn.arg.binOp)
			{
			case Instruction::BinaryOpType::AddI:
				in1out.integer = in1out.integer + in2.integer;
				break;
			case Instruction::BinaryOpType::MulI:
				in1out.integer = in1out.integer * in2.integer;
				break;
			case Instruction::BinaryOpType::SubI:
				in1out.integer = in1out.integer - in2.integer;
				break;
			case Instruction::BinaryOpType::DivI:
				in1out.integer = in1out.integer / in2.integer;
				break;
			case Instruction::BinaryOpType::Mod:
				in1out.integer = in1out.integer % in2.integer;
				break;
			case Instruction::BinaryOpType::ShlI:
				in1out.integer = in1out.integer << in2.integer;
				break;
			case Instruction::BinaryOpType::ShrI:
				in1out.integer = in1out.integer >> in2.integer;
				break;
			case Instruction::BinaryOpType::ShrU:
				in1out.integer = (unsigned int)in1out.integer >> in2.integer;
				break;
			case Instruction::BinaryOpType::AndI:
				in1out.integer = in1out.integer & in2.integer;
				break;
			case Instruction::BinaryOpType::OrI:
				in1out.integer = in1out.integer | in2.integer;
				break;
			case Instruction::BinaryOpType::XorI:
				in1out.integer = in1out.integer ^ in2.integer;
				break;
			case Instruction::BinaryOpType::EqI:
				in1out.logical = in1out.integer == in2.integer;
				break;
			case Instruction::BinaryOpType::NeI:
				in1out.logical = in1out.integer != in2.integer;
				break;
			case Instruction::BinaryOpType::LtI:
				in1out.logical = in1out.integer < in2.integer;
				break;
			case Instruction::BinaryOpType::GtI:
				in1out.logical = in1out.integer > in2.integer;
				break;
			case Instruction::BinaryOpType::LeI:
				in1out.logical = in1out.integer <= in2.integer;
				break;
			case Instruction::BinaryOpType::GeI:
				in1out.logical = in1out.integer >= in2.integer;
				break;
			case Instruction::BinaryOpType::AddF:
				in1out.floating = in1out.floating + in2.floating;
				break;
			case Instruction::BinaryOpType::MulF:
				in1out.floating = in1out.floating * in2.floating;
				break;
			case Instruction::BinaryOpType::SubF:
				in1out.floating = in1out.floating - in2.floating;
				break;
			case Instruction::BinaryOpType::DivF:
				in1out.floating = in1out.floating / in2.floating;
				break;
			case Instruction::BinaryOpType::EqF:
				in1out.logical = in1out.floating == in2.floating;
				break;
			case Instruction::BinaryOpType::NeF:
				in1out.logical = in1out.floating != in2.floating;
				break;
			case Instruction::BinaryOpType::LtF:
				in1out.logical = in1out.floating < in2.floating;
				break;
			case Instruction::BinaryOpType::GtF:
				in1out.logical = in1out.floating > in2.floating;
				break;
			case Instruction::BinaryOpType::LeF:
				in1out.logical = in1out.floating <= in2.floating;
				break;
			case Instruction::BinaryOpType::GeF:
				in1out.logical = in1out.floating >= in2.floating;
				break;
			case Instruction::BinaryOpType::And:
				in1out.logical = in1out.logical && in2.logical;
				break;
			case Instruction::BinaryOpType::Or:
				in1out.logical = in1out.logical || in2.logical;
				break;
			case Instruction::BinaryOpType::Xor:
				in1out.logical = in1out.logical ^ in2.logical;
				break;
			}
			break;
		case Instruction::Operation::WriteField:
			storage.write(in1out.reference, isn.arg.fieldOffset, in2);
			break;
		}

		if(isn.doesWriteBack())
		{
			opStack.push_back(in1out);
		}
	}

	return {};
}
