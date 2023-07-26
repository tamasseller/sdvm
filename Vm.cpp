#include "Vm.h"

Vm::Vm(Storage& s, const Program &p):
	storage(s), program(p), staticObject(s.create(p.staticType)) {}

Storage::Ref Vm::createFrame(const Program::Function &f, Storage::Ref caller, std::vector<Storage::Value> &args, size_t argCount)
{
	assert(Program::Function::Frame::offsetToLocals + argCount <= f.frame.opStackOffset);

	const auto ret = storage.create(f.frame.frameType);
	storage.write(ret, Program::Function::Frame::previousFrameOffset, caller);

	for(auto i = 0u; i < argCount; i++)
	{
		storage.write(ret, Program::Function::Frame::offsetToLocals + i, args.back());
		args.pop_back();
	}

	return ret;
}

inline Storage::Value Vm::StackState::pop(Storage& storage, Storage::Ref frame)
{
	auto ret = storage.read(frame, --stackPointer);

	if(refChainEnd == stackPointer)
	{
		refChainEnd = storage.read(frame, --stackPointer).integer;
	}

	return ret;
}

inline void Vm::StackState::pushValue(Storage& storage, Storage::Ref frame, Storage::Value value)
{
	storage.write(frame, stackPointer++, value);
}

inline void Vm::StackState::pushReference(Storage& storage, Storage::Ref frame, Storage::Value value)
{
	storage.write(frame, stackPointer++, refChainEnd);
	refChainEnd = stackPointer;
	storage.write(frame, stackPointer++, value);
}

inline void Vm::StackState::preGcFlush(Storage& storage, Storage::Ref frame)
{
	storage.write(frame, Program::Function::Frame::opStackRefChainEndOffset, refChainEnd);
}

inline void Vm::StackState::store(Storage& storage, Storage::Ref frame)
{
	preGcFlush(storage, frame);
	storage.write(frame, Program::Function::Frame::topOfStackOffset, stackPointer);
}

inline Vm::StackState Vm::StackState::load(Storage& storage, Storage::Ref frame)
{
	Vm::StackState ret;
	ret.refChainEnd = storage.read(frame, Program::Function::Frame::opStackRefChainEndOffset).integer;
	ret.stackPointer = storage.read(frame, Program::Function::Frame::topOfStackOffset).integer;
	return ret;
}

std::optional<Storage::Value> Vm::run(std::vector<Storage::Value> args)
{
	assert(!program.functions.empty());

	const auto &f = program.functions.front();

	auto currentFrame = createFrame(f, staticObject, args, args.size());

	StackState ss(f.frame.opStackOffset);

	for(auto it = f.code.begin(); it != f.code.end();)
	{
		const auto &isn = *it++;

		Storage::Value in1out, in2;

		switch(isn.popCount())
		{
			case 2:
				in2 = ss.pop(storage, currentFrame);
				[[fallthrough]];
			case 1:
				in1out = ss.pop(storage, currentFrame);
				[[fallthrough]];
			case 0:
				break;
		}

		switch(isn.op)
		{
		case Instruction::Operation::PushLiteral:
			in1out = isn.arg.literal;
			break;
		case Instruction::Operation::ReadRefLocal:
		case Instruction::Operation::ReadValLocal:
			in1out = storage.read(currentFrame, Program::Function::Frame::offsetToLocals + isn.arg.fieldOffset);
			break;
		case Instruction::Operation::ReadRefStatic:
		case Instruction::Operation::ReadValStatic:
			in1out = storage.read(staticObject, isn.arg.fieldOffset);
			break;
		case Instruction::Operation::NewObject:
			in1out = storage.create(isn.arg.type);
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
			storage.write(currentFrame, Program::Function::Frame::offsetToLocals + isn.arg.fieldOffset, in1out);
			break;
		case Instruction::Operation::WriteStatic:
			storage.write(staticObject, isn.arg.fieldOffset, in1out);
			break;
		case Instruction::Operation::ReadRefField:
		case Instruction::Operation::ReadValField:
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
		case Instruction::Operation::Ret:
			// TODO
			break;
		case Instruction::Operation::RetVal:
		{
			if(auto prevFrame = storage.read(currentFrame, Program::Function::Frame::previousFrameOffset).reference; prevFrame != staticObject)
			{
				ss = StackState::load(storage, prevFrame);
				currentFrame = prevFrame;

				ss.pushValue(storage, currentFrame, in1out);
			}
			else
			{
				return in1out;
			}
			break;
		}
		case Instruction::Operation::RetRef:
			// TODO
			break;
		case Instruction::Operation::Call:
			// TODO
			break;
		case Instruction::Operation::CallV:
			// TODO
			break;
		}

		if(isn.doesValueWriteBack())
		{
			ss.pushValue(storage, currentFrame, in1out);
		}
		else if(isn.doesReferenceWriteBack())
		{
			ss.pushReference(storage, currentFrame, in1out);
		}
	}

	return {};
}
