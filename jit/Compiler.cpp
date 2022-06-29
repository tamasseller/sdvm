#include "Compiler.h"

#include "VmTab.h"
#include "Immediate.h"
#include "Assembler.h"

static inline void summonImmediate(Assembler& a, ArmV6::LoReg target, uint32_t value)
{
	if(ImmediateFabricationPlan plan; ImmediateFabricationPlan::make(value, plan))
	{
		a.emit(ArmV6::movs(target, plan.imm));

		switch(plan.op)
		{
		case ImmediateFabricationPlan::LastOp::Add:
			a.emit(ArmV6::adds(target, plan.param));
			break;

		case ImmediateFabricationPlan::LastOp::Shift:
			a.emit(ArmV6::lsls(target, target, plan.param));
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
	uint32_t labelIdx = 0;

	Assembler::LabelInfo labels[info.nLabels + 1];
	Assembler::Label end(info.nLabels);
	Assembler a(out.start, out.length, labels, info.nLabels + 1);

	// TODO optimize leafs
	a.emit(ArmV6::mov(ArmV6::AnyReg(0), ArmV6::AnyReg(14))); 		// mov r0, lr
	a.vmTab((VMTAB_ENTER_NON_LEAF_INDEX << VMTAB_SHIFT) | fnIdx); 	// blx r9; .short <param>
	a.emit(ArmV6::add(ArmV6::AnyReg(15), ArmV6::AnyReg(14))); 		// add pc, lr

	const auto frameGapSize = 2;

	uint32_t stackDepth = info.nArgs + frameGapSize;

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

				if(isn.bin.op <= Bytecode::Instruction::BinaryOperation::Sub)
				{
					const auto idx = (int)isn.bin.op - (int)Bytecode::Instruction::BinaryOperation::Add;

					static constexpr const ArmV6::Reg3Op opLookup[] =
					{
						/* Add */ ArmV6::Reg3Op::ADDREG,
						/* Sub */ ArmV6::Reg3Op::SUBREG,
					};

					a.emit(ArmV6::fmtReg3(opLookup[idx], nd.idx, nd.idx, m.idx));
				}
				else if(isn.bin.op <= Bytecode::Instruction::BinaryOperation::Mul)
				{
					const auto idx = (int)isn.bin.op - (int)Bytecode::Instruction::BinaryOperation::And;

					static constexpr const ArmV6::Reg2Op opLookup[] =
					{
						/* And */ ArmV6::Reg2Op::AND,
						/* Ior */ ArmV6::Reg2Op::ORR,
						/* Xor */ ArmV6::Reg2Op::EOR,
						/* Lsh */ ArmV6::Reg2Op::LSL,
						/* Rsh */ ArmV6::Reg2Op::LSR,
						/* Ash */ ArmV6::Reg2Op::ASR,
						/* Mul */ ArmV6::Reg2Op::MUL,
					};

					a.emit(ArmV6::fmtReg2(opLookup[idx], nd.idx, m.idx));
				}
				else
				{
					/* GCOV_EXCL_START */
					assert(isn.bin.op <= Bytecode::Instruction::BinaryOperation::Mod);
					const auto idx = (int)isn.bin.op - (int)Bytecode::Instruction::BinaryOperation::Div;

					static constexpr const uint16_t controlValueLookup[] =
					{
						// TODO NYET
					};

					a.vmTab(controlValueLookup[idx]);
					/* GCOV_EXCL_STOP */
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

				assert((size_t)isn.cond.cond < sizeof(condLookup));

				a.emit(ArmV6::condBranch(condLookup[(size_t)isn.cond.cond], Assembler::Label(isn.cond.targetIdx)));
				stackDepth -= 2; // TODO reconcile laziness: check target and save if first jump, manifest stored state if not.
				break;
			}
			case Bytecode::Instruction::OperationGroup::Jump:
			{
				a.emit(ArmV6::b(Assembler::Label(isn.jump.targetIdx)));

				// TODO reconcile laziness: check target and save if first jump, manifest stored state if not.
				break;
			}
			case Bytecode::Instruction::OperationGroup::Label:
			{
				stackDepth += isn.label.stackAdjustment;
				a.pin(Assembler::Label(labelIdx++));
				break;
			}
			case Bytecode::Instruction::OperationGroup::Move:
			{
				const auto r = ArmV6::LoReg(0);

				switch(isn.move.op)
				{
					case Bytecode::Instruction::MoveOperation::Pull:
					{
						assert(isn.move.param < stackDepth);
						ArmV6::Uoff<2, 8> offset = (stackDepth - isn.move.param) << 2;
						a.emit(ArmV6::ldrSp(r, offset));
						a.emit(ArmV6::push(ArmV6::LoRegs{}.add(r)));
						stackDepth++;
						break;
					}
					case Bytecode::Instruction::MoveOperation::Shove:
					{
						a.emit(ArmV6::pop(ArmV6::LoRegs{}.add(r)));
						stackDepth--;
						assert(isn.move.param < stackDepth);

						ArmV6::Uoff<2, 8> offset = (stackDepth - isn.move.param) << 2;
						a.emit(ArmV6::strSp(r, offset));
						break;
					}
					case Bytecode::Instruction::MoveOperation::Drop:
					{
						assert(isn.move.param < stackDepth);
						stackDepth -= isn.move.param;
						a.emit(ArmV6::incrSp(isn.move.param << 2));
						break;
					}
				}

				break;
			}
			case Bytecode::Instruction::OperationGroup::Call:
			{
				// TODO do all lazy calculations here and write result in a PCS compatible manner.

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
				// TODO do all lazy calculations here and write result in a PCS compatible manner.
				a.emit(ArmV6::b(end));
				stackDepth -= info.nRet;
				break;
			}
		}
	}

	assert(labelIdx == info.nLabels);

	a.pin(end);
	a.vmTab((VMTAB_LEAVE_NON_LEAF_INDEX << VMTAB_SHIFT) | info.nRet);

	return a.assemble();
}
