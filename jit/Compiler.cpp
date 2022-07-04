#include "Compiler.h"

#include "VmTab.h"
#include "RegisterAllocator.h"

uint16_t *Compiler::compile(uint16_t fnIdx, const Output& out, const Bytecode::FunctionInfo& info, Bytecode::InstructionStreamReader& reader)
{
	uint32_t labelIdx = 0;

	Assembler::LabelInfo labels[info.nLabels + 1];
	Assembler::Label end(info.nLabels);
	Assembler a(out.start, out.length, labels, info.nLabels + 1);

	// TODO optimize leafs
	const auto startPtr = a.getPtr();
	a.emit(ArmV6::mov(ArmV6::AnyReg(0), ArmV6::AnyReg(14)));
	a.vmTab((VMTAB_ENTER_NON_LEAF_INDEX << VMTAB_SHIFT) | fnIdx);

	const auto resumePtr = a.getPtr();
	a.emit(ArmV6::add(ArmV6::AnyReg(15), ArmV6::AnyReg(14)));

	assert((char*)resumePtr - (char*)startPtr == RESUME_OFFSET);

	// 	const auto incSpPtr1 = a.getPtr(); a.emit(ArmV6::nop());
	// 	const auto pushCsRegPtr = a.getPtr(); a.emit(ArmV6::nop());
	// 	const auto incSpPtr2 = a.getPtr(); a.emit(ArmV6::nop());

	RegisterAllocator ra(info.nArgs);

	bool emitRetBranch = false;

	for(Bytecode::Instruction isn; reader(isn);)
	{
		if(emitRetBranch)
		{
			emitRetBranch = false;
			a.emit(ArmV6::b(end));
		}
		switch(isn.g)
		{
			case Bytecode::Instruction::OperationGroup::Immediate:
			{
				ra.pushImmediate(a, isn.imm.value);
				break;
			}

			case Bytecode::Instruction::OperationGroup::Binary:
			{
				const auto m = ra.consume(a);
				const auto nd = ra.consume(a);
				const auto d = ra.acquire(a);
				assert(d.idx == nd.idx);

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

				break;
			}
			case Bytecode::Instruction::OperationGroup::Conditional:
			{
				const auto m = ra.consume(a);
				const auto n = ra.consume(a);

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

				break;
			}
			case Bytecode::Instruction::OperationGroup::Jump:
			{
				a.emit(ArmV6::b(Assembler::Label(isn.jump.targetIdx)));

				// TODO reconcile laziness: check target and save if state is not set, apply stored state if it is.
				break;
			}
			case Bytecode::Instruction::OperationGroup::Label:
			{
				// TODO reconcile laziness: check target and store state if not set already, apply stored state if it is.
				// TODO apply isn.label.stackAdjustment;

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
						ra.pull(a, isn.move.param);
						break;
					}
					case Bytecode::Instruction::MoveOperation::Shove:
					{
						ra.shove(a, isn.move.param);
						break;
					}
					case Bytecode::Instruction::MoveOperation::Drop:
					{
						ra.drop(a, isn.move.param);
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

//				assert(isn.call.nArgs <= stackDepth);
//
//				stackDepth += isn.call.nRet - isn.call.nArgs;

				break;
			}
			case Bytecode::Instruction::OperationGroup::Return:
			{
				// TODO do all lazy calculations here and write result in a PCS compatible manner.
				emitRetBranch = true;

//				assert(info.nRet <= stackDepth);
//				stackDepth -= info.nRet;
				break;
			}
		}
	}

	assert(labelIdx == info.nLabels);

	a.pin(end);
	a.vmTab((VMTAB_LEAVE_NON_LEAF_INDEX << VMTAB_SHIFT) | info.nRet);

	return a.assemble();
}
