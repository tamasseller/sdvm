#include "Vm.h"

#include "object/Value.h"

using namespace vm;

static constexpr auto previousFrameOffset = 0;
static constexpr auto topOfStackOffset = 1;
static constexpr auto offsetToLocals = 2;

inline Vm::ExecutionState Vm::ExecutionState::enter(Vm& vm, uint32_t fnIdx, obj::Reference caller, std::vector<obj::Value> &args, size_t argCount)
{
	assert(fnIdx < vm.program.functions.size());

	const auto &fun = vm.program.functions[fnIdx];

	assert(fun.frameTypeIndex < vm.program.types.size());

	Vm::ExecutionState ret;
/*	ret.frame = vm.storage.create(vm.program, fun.frameTypeIndex),
	ret.stackPointer = frame.opStackOffset,
	ret.functionIndex = fnIdx,
	ret.isnIt = fun.code.cbegin(),
	ret.end = fun.code.cend(),

	vm.storage.write(vm.program, ret.frame, prog::Frame::previousFrameOffset, caller);

	assert(prog::Frame::offsetToLocals + argCount <= frame.opStackOffset);

	for(auto i = 0u; i < argCount; i++)
	{
		vm.storage.write(vm.program, ret.frame, prog::Frame::offsetToLocals + i, args.back());
		args.pop_back();
	}*/

	return ret;
}

inline obj::Reference Vm::ExecutionState::getCallerFrame(Vm& vm) {
//	return vm.storage.read(vm.program, frame, prog::Frame::previousFrameOffset).reference;
}

inline bool Vm::ExecutionState::fetch(prog::Instruction& isn)
{
	if(isnIt != end)
	{
		isn = *isnIt++;
		return true;
	}

	return false;
}

inline void Vm::ExecutionState::jump(Vm& vm, uint32_t offset)
{
	assert(functionIndex < vm.program.functions.size());
	const auto &fun = vm.program.functions[functionIndex];

	assert(offset < fun.code.size());
	isnIt = fun.code.begin() + offset;
}

inline obj::Value Vm::ExecutionState::pop(Vm& vm) {
	return vm.storage.read(vm.program, frame, --stackPointer);
}

inline void Vm::ExecutionState::push(Vm& vm, obj::Value value) {
	vm.storage.write(vm.program, frame, stackPointer++, value);
}

inline obj::Value Vm::ExecutionState::readLocal(Vm& vm, uint32_t offset) {
//	return vm.storage.read(vm.program, frame, prog::Frame::offsetToLocals + offset);
}

inline void Vm::ExecutionState::writeLocal(Vm& vm, uint32_t offset, obj::Value value) {
//	vm.storage.write(vm.program, frame, prog::Frame::offsetToLocals + offset, value);
}

inline void Vm::ExecutionState::suspend(Vm& vm)
{
	push(vm, functionIndex);
	const auto offset = isnIt - vm.program.functions[functionIndex].code.cbegin();
	push(vm, (uint32_t)offset);
//	vm.storage.write(vm.program, frame, prog::Frame::topOfStackOffset, stackPointer);
}

inline Vm::ExecutionState Vm::ExecutionState::resume(Vm& vm, obj::Reference frame)
{
	Vm::ExecutionState ret;
	ret.frame = frame;

//	ret.stackPointer = vm.storage.read(vm.program, frame, prog::Frame::topOfStackOffset).integer;
	const auto offset = ret.pop(vm).integer;

	ret.functionIndex = ret.pop(vm).integer;

	const auto &fun = vm.program.functions[ret.functionIndex];
	ret.isnIt = fun.code.cbegin() + offset;
	ret.end = fun.code.cend();

	return ret;
}

Vm::Vm(obj::Storage& storage, const prog::Program &p): storage(storage), program(p)
{
	assert(!p.types.empty());
	staticObject = storage.create(p, 0);
}

std::optional<obj::Value> Vm::run(std::vector<obj::Value> args)
{
	assert(!program.functions.empty());

	auto ss = ExecutionState::enter(*this, 0, staticObject, args, args.size());

	while(true)
	{
		prog::Instruction isn;
		auto fetchOk = ss.fetch(isn);
		assert(fetchOk);

		obj::Value in1out, in2;

		switch(isn.popCount())
		{
			case 2:
				in2 = ss.pop(*this);
				[[fallthrough]];
			case 1:
				in1out = ss.pop(*this);
				[[fallthrough]];
			case 0:
				break;
		}

		switch(isn.op)
		{
		case prog::Instruction::Operation::PushLiteral:
			in1out = isn.arg.literal;
			break;
		case prog::Instruction::Operation::ReadLocal:
			in1out = ss.readLocal(*this, isn.arg.index);
			break;
		case prog::Instruction::Operation::ReadStatic:
			in1out = storage.read(program, staticObject, isn.arg.index);
			break;
		case prog::Instruction::Operation::NewObject:
			assert(isn.arg.index < program.types.size());
			in1out = storage.create(program, isn.arg.index);
			break;
		case prog::Instruction::Operation::Jump:
			ss.jump(*this, isn.arg.index);
			break;
		case prog::Instruction::Operation::Cond:
			if(in1out.logical)
			{
				ss.jump(*this, isn.arg.index);
			}
			break;
		case prog::Instruction::Operation::WriteLocal:
			ss.writeLocal(*this, isn.arg.index, in1out);
			break;
		case prog::Instruction::Operation::WriteStatic:
			storage.write(program, staticObject, isn.arg.index, in1out);
			break;
		case prog::Instruction::Operation::ReadField:
			in1out = storage.read(program, in1out.reference, isn.arg.index);
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
			case prog::Instruction::BinaryOpType::AndL:
				in1out.logical = in1out.logical && in2.logical;
				break;
			case prog::Instruction::BinaryOpType::OrL:
				in1out.logical = in1out.logical || in2.logical;
				break;
			}
			break;
		case prog::Instruction::Operation::WriteField:
			storage.write(program, in1out.reference, isn.arg.index, in2);
			break;
		case prog::Instruction::Operation::Ret:
			// TODO
			break;
		case prog::Instruction::Operation::RetVal:
		{
			if(auto prevFrame = ss.getCallerFrame(*this); prevFrame != staticObject)
			{
				ss = ExecutionState::resume(*this, prevFrame);
				ss.push(*this, in1out);
			}
			else
			{
				return in1out;
			}
			break;
		}
		case prog::Instruction::Operation::Call:
			// TODO
			break;
		case prog::Instruction::Operation::CallV:
			// TODO
			break;
		}

		if(isn.doesWriteBack())
		{
			ss.push(*this, in1out);
		}
	}

	return {};
}
