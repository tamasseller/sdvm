#include "Vm.h"

using namespace vm;

Vm::Vm(obj::Storage& s, const prog::Program &p):
	storage(s), program(p), staticObject(s.create(p.staticType)) {}

obj::Reference Vm::createFrame(const prog::Function &f, obj::Reference caller, std::vector<obj::Value> &args, size_t argCount)
{
	assert(prog::Frame::offsetToLocals + argCount <= f.frame.opStackOffset);

	const auto ret = storage.create(f.frame.frameType);
	storage.write(ret, prog::Frame::previousFrameOffset, caller);

	for(auto i = 0u; i < argCount; i++)
	{
		storage.write(ret, prog::Frame::offsetToLocals + i, args.back());
		args.pop_back();
	}

	return ret;
}

inline obj::Value Vm::StackState::pop(obj::Storage& storage, obj::Reference frame)
{
	auto ret = storage.read(frame, --stackPointer);

	if(refChainEnd == stackPointer)
	{
		refChainEnd = storage.read(frame, --stackPointer).integer;
	}

	return ret;
}

inline void Vm::StackState::pushValue(obj::Storage& storage, obj::Reference frame, obj::Value value)
{
	storage.write(frame, stackPointer++, value);
}

inline void Vm::StackState::pushReference(obj::Storage& storage, obj::Reference frame, obj::Value value)
{
	storage.write(frame, stackPointer++, refChainEnd);
	refChainEnd = stackPointer;
	storage.write(frame, stackPointer++, value);
}

inline void Vm::StackState::preGcFlush(obj::Storage& storage, obj::Reference frame)
{
	storage.write(frame, prog::Frame::opStackRefChainEndOffset, refChainEnd);
}

inline void Vm::StackState::store(obj::Storage& storage, obj::Reference frame)
{
	preGcFlush(storage, frame);
	storage.write(frame, prog::Frame::topOfStackOffset, stackPointer);
}

inline Vm::StackState Vm::StackState::load(obj::Storage& storage, obj::Reference frame)
{
	Vm::StackState ret;
	ret.refChainEnd = storage.read(frame, prog::Frame::opStackRefChainEndOffset).integer;
	ret.stackPointer = storage.read(frame, prog::Frame::topOfStackOffset).integer;
	return ret;
}

std::optional<obj::Value> Vm::run(std::vector<obj::Value> args)
{
	assert(!program.functions.empty());

	const auto &f = program.functions.front();

	auto currentFrame = createFrame(f, staticObject, args, args.size());

	StackState ss(f.frame.opStackOffset);

	for(auto it = f.code.begin(); it != f.code.end();)
	{
		const auto &isn = *it++;

		obj::Value in1out, in2;

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
		case prog::Instruction::Operation::PushLiteral:
			in1out = isn.arg.literal;
			break;
		case prog::Instruction::Operation::ReadRefLocal:
		case prog::Instruction::Operation::ReadValLocal:
			in1out = storage.read(currentFrame, prog::Frame::offsetToLocals + isn.arg.fieldOffset);
			break;
		case prog::Instruction::Operation::ReadRefStatic:
		case prog::Instruction::Operation::ReadValStatic:
			in1out = storage.read(staticObject, isn.arg.fieldOffset);
			break;
		case prog::Instruction::Operation::NewObject:
			in1out = storage.create(isn.arg.type);
			break;
		case prog::Instruction::Operation::Jump:
			it = f.code.begin() + isn.arg.jumpTarget;
			break;
		case prog::Instruction::Operation::Cond:
			if(in1out.logical)
			{
				it = f.code.begin() + isn.arg.jumpTarget;
			}
			break;
		case prog::Instruction::Operation::WriteLocal:
			storage.write(currentFrame, prog::Frame::offsetToLocals + isn.arg.fieldOffset, in1out);
			break;
		case prog::Instruction::Operation::WriteStatic:
			storage.write(staticObject, isn.arg.fieldOffset, in1out);
			break;
		case prog::Instruction::Operation::ReadRefField:
		case prog::Instruction::Operation::ReadValField:
			in1out = storage.read(in1out.reference, isn.arg.fieldOffset);
			break;
		case prog::Instruction::Operation::Unary:
			switch(isn.arg.unOp)
			{
			case prog::Instruction::UnaryOpType::Not:
				in1out.logical = !in1out.logical;
				break;
			case prog::Instruction::UnaryOpType::BitwiseNegate:
				in1out.integer = ~in1out.integer;
				break;
			}
			break;
		case prog::Instruction::Operation::Binary:
			switch(isn.arg.binOp)
			{
			case prog::Instruction::BinaryOpType::AddI:
				in1out.integer = in1out.integer + in2.integer;
				break;
			case prog::Instruction::BinaryOpType::MulI:
				in1out.integer = in1out.integer * in2.integer;
				break;
			case prog::Instruction::BinaryOpType::SubI:
				in1out.integer = in1out.integer - in2.integer;
				break;
			case prog::Instruction::BinaryOpType::DivI:
				in1out.integer = in1out.integer / in2.integer;
				break;
			case prog::Instruction::BinaryOpType::Mod:
				in1out.integer = in1out.integer % in2.integer;
				break;
			case prog::Instruction::BinaryOpType::ShlI:
				in1out.integer = in1out.integer << in2.integer;
				break;
			case prog::Instruction::BinaryOpType::ShrI:
				in1out.integer = in1out.integer >> in2.integer;
				break;
			case prog::Instruction::BinaryOpType::ShrU:
				in1out.integer = (unsigned int)in1out.integer >> in2.integer;
				break;
			case prog::Instruction::BinaryOpType::AndI:
				in1out.integer = in1out.integer & in2.integer;
				break;
			case prog::Instruction::BinaryOpType::OrI:
				in1out.integer = in1out.integer | in2.integer;
				break;
			case prog::Instruction::BinaryOpType::XorI:
				in1out.integer = in1out.integer ^ in2.integer;
				break;
			case prog::Instruction::BinaryOpType::EqI:
				in1out.logical = in1out.integer == in2.integer;
				break;
			case prog::Instruction::BinaryOpType::NeI:
				in1out.logical = in1out.integer != in2.integer;
				break;
			case prog::Instruction::BinaryOpType::LtI:
				in1out.logical = in1out.integer < in2.integer;
				break;
			case prog::Instruction::BinaryOpType::GtI:
				in1out.logical = in1out.integer > in2.integer;
				break;
			case prog::Instruction::BinaryOpType::LeI:
				in1out.logical = in1out.integer <= in2.integer;
				break;
			case prog::Instruction::BinaryOpType::GeI:
				in1out.logical = in1out.integer >= in2.integer;
				break;
			case prog::Instruction::BinaryOpType::AddF:
				in1out.floating = in1out.floating + in2.floating;
				break;
			case prog::Instruction::BinaryOpType::MulF:
				in1out.floating = in1out.floating * in2.floating;
				break;
			case prog::Instruction::BinaryOpType::SubF:
				in1out.floating = in1out.floating - in2.floating;
				break;
			case prog::Instruction::BinaryOpType::DivF:
				in1out.floating = in1out.floating / in2.floating;
				break;
			case prog::Instruction::BinaryOpType::EqF:
				in1out.logical = in1out.floating == in2.floating;
				break;
			case prog::Instruction::BinaryOpType::NeF:
				in1out.logical = in1out.floating != in2.floating;
				break;
			case prog::Instruction::BinaryOpType::LtF:
				in1out.logical = in1out.floating < in2.floating;
				break;
			case prog::Instruction::BinaryOpType::GtF:
				in1out.logical = in1out.floating > in2.floating;
				break;
			case prog::Instruction::BinaryOpType::LeF:
				in1out.logical = in1out.floating <= in2.floating;
				break;
			case prog::Instruction::BinaryOpType::GeF:
				in1out.logical = in1out.floating >= in2.floating;
				break;
			case prog::Instruction::BinaryOpType::And:
				in1out.logical = in1out.logical && in2.logical;
				break;
			case prog::Instruction::BinaryOpType::Or:
				in1out.logical = in1out.logical || in2.logical;
				break;
			case prog::Instruction::BinaryOpType::Xor:
				in1out.logical = in1out.logical ^ in2.logical;
				break;
			}
			break;
		case prog::Instruction::Operation::WriteField:
			storage.write(in1out.reference, isn.arg.fieldOffset, in2);
			break;
		case prog::Instruction::Operation::Ret:
			// TODO
			break;
		case prog::Instruction::Operation::RetVal:
		{
			if(auto prevFrame = storage.read(currentFrame, prog::Frame::previousFrameOffset).reference; prevFrame != staticObject)
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
		case prog::Instruction::Operation::RetRef:
			// TODO
			break;
		case prog::Instruction::Operation::Call:
			// TODO
			break;
		case prog::Instruction::Operation::CallV:
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
