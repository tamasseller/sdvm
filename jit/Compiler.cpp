#include "Compiler.h"

#include "VmTab.h"
#include "Assembler.h"
#include "Immediate.h"

static inline void summonImmediate(Assembler& a, ArmV6::LoReg target, uint32_t value)
{
	if(ImmediateFabricationPlan plan; ImmediateFabricationPlan::make(value, plan))
	{
		a.emit(ArmV6::mov(target, plan.imm));

		switch(plan.op)
		{
		case ImmediateFabricationPlan::LastOp::Add:
			a.emit(ArmV6::adds(target, plan.param));
			break;

		case ImmediateFabricationPlan::LastOp::Shift:
			a.emit(ArmV6::lsls(target, plan.param));
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

	Assembler::LabelInfo labels[info.nLabels + 1];
	Assembler::Label end(info.nLabels);
	Assembler a(out.start, out.length, labels, info.nLabels);

	// TODO optimize leafs
	assert(info.hasNonTailCall);
	a.emit(ArmV6::mov(ArmV6::AnyReg(0), ArmV6::AnyReg(14))); // mov r0, lr
	a.emit(ArmV6::blx(ArmV6::AnyReg(9)));
	a.vmTab((VMTAB_ENTER_NON_LEAF_INDEX << VMTAB_SHIFT) | fnIdx);
	a.emit(ArmV6::add(ArmV6::AnyReg(15), ArmV6::AnyReg(14))); // add pc, lr

	const auto frameGapSize = 2;

	for(Bytecode::Instruction isn; reader(isn);)
	{
		switch(isn.g)
		{
			case Bytecode::Instruction::OperationGroup::Immediate:
			{
				const auto value = isn.imm.value;
				const auto target = ArmV6::LoReg(0); // TODO lazy pushing and allocation

				summonImmediate(a, target, value);

				a.emit(ArmV6::push(ArmV6::LoRegs{}.add(target))); // TODO lazy pushing
				stackDepth++;
				break;
			}

			case Bytecode::Instruction::OperationGroup::Binary:
			{
				const auto m = ArmV6::LoReg(0);
				const auto nd = ArmV6::LoReg(1); // TODO lazy pushing and allocation

				a.emit(ArmV6::pop(ArmV6::LoRegs{}.add(m).add(nd))); // TODO lazy

				switch(isn.bin.op)
				{
					case Bytecode::Instruction::BinaryOperation::Add:
					{
						a.emit(ArmV6::adds(nd, nd, m));
						break;
					}
					case Bytecode::Instruction::BinaryOperation::Sub:
					{
						a.emit(ArmV6::subs(nd, nd, m));

						break;
					}
					case Bytecode::Instruction::BinaryOperation::Mul:
					{
						a.emit(ArmV6::muls(nd, m));
						break;
					}
					case Bytecode::Instruction::BinaryOperation::Div:
					{
						// TODO add vmcall via "bx r9; .short <id>"
						break;
					}
					case Bytecode::Instruction::BinaryOperation::Mod:
					{
						// TODO add vmcall via "bx r9; .short <id>"
						break;
					}
					case Bytecode::Instruction::BinaryOperation::And:
					{
						a.emit(ArmV6::ands(nd, m));
						break;
					}
					case Bytecode::Instruction::BinaryOperation::Ior:
					{
						a.emit(ArmV6::orrs(nd, m));
						break;
					}
					case Bytecode::Instruction::BinaryOperation::Xor:
					{
						a.emit(ArmV6::eors(nd, m));
						break;
					}
					case Bytecode::Instruction::BinaryOperation::Lsh:
					{
						a.emit(ArmV6::lsls(nd, m));
						break;
					}
					case Bytecode::Instruction::BinaryOperation::Rsh:
					{
						a.emit(ArmV6::lsrs(nd, m));
						break;
					}
					case Bytecode::Instruction::BinaryOperation::Ash:
					{
						a.emit(ArmV6::asrs(nd, m));
						break;
					}
				}

				a.emit(ArmV6::push(ArmV6::LoRegs{}.add(nd))); // TODO lazy pushing
				stackDepth -= 1;
				break;
			}
			case Bytecode::Instruction::OperationGroup::Conditional:
			{
				const auto m = ArmV6::LoReg(0); // TODO lazy pushing and allocation
				const auto n = ArmV6::LoReg(1);

				a.emit(ArmV6::pop(ArmV6::LoRegs{}.add(m).add(n))); // TODO lazy

				a.emit(ArmV6::cmp(n, m));

				static constexpr const ArmV6::Condition condLookup[] =
				{
					/* Equal              */ ArmV6::Condition::EQ,
					/* NotEqual           */ ArmV6::Condition::NE,
					/* UnsignedGreater    */ ArmV6::Condition::HI,
					/* UnsignedNotGreater */ ArmV6::Condition::LS,
					/* UnsignedLess       */ ArmV6::Condition::LO,
					/* UnsignedNotLess    */ ArmV6::Condition::HS,
					/* SignedGreater      */ ArmV6::Condition::GT,
					/* SignedNotGreater   */ ArmV6::Condition::LE,
					/* SignedLess         */ ArmV6::Condition::LT,
					/* SignedNotLess      */ ArmV6::Condition::GE
				};

				assert(isn.cond.cond < sizeof(condLookup));

				a.emit(ArmV6::condBranch(condLookup[isn.cond.cond], Assembler::Label(isn.cond.targetIdx)));
				stackDepth -= 2; // TODO reconcile laziness: check target and save if first jump, manifest stored state if not.
				break;
			}
			case Bytecode::Instruction::OperationGroup::Jump:
			{
				a.emit(ArmV6::b(Assembler::Label(isn.cond.targetIdx)));

				// TODO reconcile laziness: check target and save if first jump, manifest stored state if not.
				break;
			}
			case Bytecode::Instruction::OperationGroup::Label:
			{
				a.pin(Assembler::Label(labelIdx++));
				break;
			}
			case Bytecode::Instruction::OperationGroup::Move:
			{
				const auto r = ArmV6::LoReg(0);
				ArmV6::Uoff<2, 8> offset;

				switch(isn.move.target)
				{
					case Bytecode::Instruction::DupTarget::Arg:
					{
						assert(isn.move.idx < stackDepth);
						offset = (stackDepth - isn.move.idx) << 2;
						break;
					}
					case Bytecode::Instruction::DupTarget::Stack:
					{
						offset = (stackDepth + frameGapSize + isn.move.idx) << 2;
						break;
					}
				}

				switch(isn.move.dir)
				{
					case Bytecode::Instruction::DupDirection::In:
					{
						a.emit(ArmV6::ldrSp(r, offset));
						a.emit(ArmV6::push(ArmV6::LoRegs{}.add(r)));
						stackDepth++;
						break;
					}
					case Bytecode::Instruction::DupDirection::Out:
					{
						a.emit(ArmV6::pop(ArmV6::LoRegs{}.add(r)));
						a.emit(ArmV6::strSp(r, offset));
						stackDepth--;
						break;
					}
				}

				break;
			}
			case Bytecode::Instruction::OperationGroup::Call:
			{
				// TODO flush all lazy calculations here such that it is PCS compatible.

				const auto r = ArmV6::LoReg(0);	// Must be r0 (TODO move to r0 if not).
				const auto s = ArmV6::LoReg(1);
				a.emit(ArmV6::pop(ArmV6::LoRegs{}.add(r)));
				a.emit(ArmV6::add(r, ArmV6::AnyReg(10)));
				a.emit(ArmV6::ldr(s, r, ArmV6::Uoff<2, 5>(0)));
				a.emit(ArmV6::blx(s));
				stackDepth += isn.call.nRet - isn.call.nArgs;
				break;
			}
			case Bytecode::Instruction::OperationGroup::Return:
			{
				// TODO flush all lazy calculations here such that it is PCS compatible.
				a.emit(ArmV6::b(end));
				break;
			}
		}
	}

	assert(labelIdx == info.nLabels);

	a.pin(end);

	writeFunctionEpilogue(a, info.nRet); // TODO optimize leafs
	return a.assemble();
}
