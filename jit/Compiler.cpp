#include "Compiler.h"

#include "Assembler.h"
#include "Immediate.h"

static inline void writeFunctionPrologue(Assembler& a)
{

}

static inline void writeFunctionEpilogue(Assembler& a)
{

}

static inline void summonImmediate(Assembler& a, ArmV6::LoReg target, uint32_t value)
{
	if(ImmediateFabricationPlan plan; ImmediateFabricationPlan::make(value, plan))
	{
		a.emit(ArmV6::mov(target, plan.imm));

		if(plan.shift)
		{
			a.emit(ArmV6::lsls(target, plan.shift));
		}

		switch(plan.op)
		{
		case ImmediateFabricationPlan::LastOp::Add:
			a.emit(ArmV6::adds(target, plan.param));
			break;

		case ImmediateFabricationPlan::LastOp::Sub:
			a.emit(ArmV6::subs(target, plan.param));
			break;

		case ImmediateFabricationPlan::LastOp::Not:
			a.emit(ArmV6::mvns(target, target));
			break;
		}
	}
	else
	{
		a.emit(ArmV6::ldrPc(target, a.literal(value)));
	}
}

uint16_t *Compiler::compile(uint16_t fnIdx, const Output& out, const Bytecode::FunctionInfo& info, Bytecode::InstructionStreamReader& reader)
{
	uint32_t stackDepth = 0;
	uint32_t labelIdx = 0;

	Assembler::LabelInfo labels[info.nLabels];
	Assembler a(out.start, out.length, labels, info.nLabels);

	writeFunctionPrologue(a); // TODO optimize leafs

	for(Bytecode::Instruction isn; reader(isn);)
	{
		switch(isn.g)
		{
		case Bytecode::Instruction::OperationGroup::Immediate:
		{
			const auto value = isn.imm.value;
			const auto target = ArmV6::LoReg(0); // TODO handle allocation (and lazy push if needed)

			summonImmediate(a, target, value);

			a.emit(ArmV6::pushWoLr(ArmV6::LoRegs{}.add(target))); // TODO lazy pushing
			break;
		}

		case Bytecode::Instruction::OperationGroup::Binary:
			break;
		case Bytecode::Instruction::OperationGroup::Conditional:
			break;
		case Bytecode::Instruction::OperationGroup::Jump:
			break;
		case Bytecode::Instruction::OperationGroup::Label:
			break;
		}
	}

	writeFunctionEpilogue(a); // TODO optimize leafs
	return a.assemble();
}


