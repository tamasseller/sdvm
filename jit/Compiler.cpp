#include "Analyzer.h"
#include "Compiler.h"
#include "Assembler.h"

#include "VmTab.h"
#include "RegisterAllocator.h"

static inline void harmonizeState(Assembler& a, RegisterAllocator &ra, Assembler::LabelInfo& li)
{
	ra.flushDeferred(a);

	if(li.reached)
	{
		ra.applyState(a, li.raState);
	}
	else
	{
		li.reached = true;
		li.raState = ra.getState();
	}
}

static inline void arrangeReturn(Assembler& a, RegisterAllocator &ra, uint16_t nRet)
{
	// TODO free up arg register

	for(int i = 0; i < nRet; i++)
	{
		ra.consume(a); // TODO place in regs/push
	}
}


uint16_t *Compiler::compile(uint16_t fnIdx, const Output& out, Bytecode::FunctionReader *reader)
{
	uint32_t labelIdx = 0;

	Analyzer::Result aRes;

	if(!Analyzer::analyze(aRes, reader))
	{
		return nullptr;
	}

	const Bytecode::FunctionInfo info = reader->reset();

	Assembler::LabelInfo labels[aRes.nLabels + 1];
	Assembler::Label end(aRes.nLabels);
	Assembler a(out.start, out.length, labels, aRes.nLabels + 1);

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

	for(Bytecode::Instruction isn; reader->read(isn);)
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
				ra.pushImmediate(a, isn.immValue);
				break;
			}

			case Bytecode::Instruction::OperationGroup::Binary:
			{
				if(isn.binOp == Bytecode::Instruction::BinaryOperation::Add)
				{
					if(uint32_t imm; ra.getTosImm(imm))
					{
						if(imm < 32)
						{
							ra.drop(a, 1);
							const auto n = ra.consume(a);
							const auto d = ra.acquire(a);

							a.emit(ArmV6::adds(d, n, imm));
							break;
						}
						else if(imm < 256)
						{
							ra.drop(a, 1);
							const auto nd = ra.replace(a);

							a.emit(ArmV6::adds(nd, imm));
							break;
						}
					}

					const auto m = ra.consume(a);

					if(uint32_t imm; ra.getTosImm(imm) && imm < 32)
					{
						ra.drop(a, 1);
						const auto d = ra.acquire(a);

						a.emit(ArmV6::adds(d, m, imm));
						break;
					}

					const auto n = ra.consume(a);
					const auto d = ra.acquire(a);

					a.emit(ArmV6::adds(d, n, m));
				}
				else if(isn.binOp == Bytecode::Instruction::BinaryOperation::Sub)
				{
					if(uint32_t imm; ra.getTosImm(imm))
					{
						if(imm < 32)
						{
							ra.drop(a, 1);
							const auto n = ra.consume(a);
							const auto d = ra.acquire(a);

							a.emit(ArmV6::subs(d, n, imm));
							break;
						}
						else if(imm < 256)
						{
							ra.drop(a, 1);
							const auto nd = ra.replace(a);

							a.emit(ArmV6::subs(nd, imm));
							break;
						}
					}

					const auto m = ra.consume(a);

					if(uint32_t imm; ra.getTosImm(imm) && imm == 0)
					{
						ra.drop(a, 1);
						const auto d = ra.acquire(a);

						a.emit(ArmV6::negs(d, m));
						break;
					}

					const auto n = ra.consume(a);
					const auto d = ra.acquire(a);

					a.emit(ArmV6::subs(d, n, m));
				}
				else if(isn.binOp <= Bytecode::Instruction::BinaryOperation::Ash)
				{
					const auto idx = (int)isn.binOp - (int)Bytecode::Instruction::BinaryOperation::Lsh;

					if(uint32_t imm; ra.getTosImm(imm) && imm < 32)
					{
						ra.drop(a, 1);
						const auto m = ra.consume(a);
						const auto d = ra.acquire(a);

						static constexpr const ArmV6::Imm5Op opLookup[] =
						{
							/* Lsh */ ArmV6::Imm5Op::LSL,
							/* Rsh */ ArmV6::Imm5Op::LSR,
							/* Ash */ ArmV6::Imm5Op::ASR,
						};

						a.emit(ArmV6::fmtImm5(opLookup[idx], d.idx, m.idx, imm));
					}
					else
					{
						const auto m = ra.consume(a);
						const auto nd = ra.replace(a);

						static constexpr const ArmV6::Reg2Op opLookup[] =
						{
							/* Lsh */ ArmV6::Reg2Op::LSL,
							/* Rsh */ ArmV6::Reg2Op::LSR,
							/* Ash */ ArmV6::Reg2Op::ASR,
						};

						a.emit(ArmV6::fmtReg2(opLookup[idx], nd.idx, m.idx));
					}

				}
				else if(isn.binOp <= Bytecode::Instruction::BinaryOperation::Mul)
				{
					const auto m = ra.consume(a);
					const auto nd = ra.replace(a);

					const auto idx = (int)isn.binOp - (int)Bytecode::Instruction::BinaryOperation::And;

					static constexpr const ArmV6::Reg2Op opLookup[] =
					{
						/* And */ ArmV6::Reg2Op::AND,
						/* Ior */ ArmV6::Reg2Op::ORR,
						/* Xor */ ArmV6::Reg2Op::EOR,
						/* Mul */ ArmV6::Reg2Op::MUL,
					};

					a.emit(ArmV6::fmtReg2(opLookup[idx], nd.idx, m.idx));
				}
				else
				{
					/* GCOV_EXCL_START */
					assert(isn.binOp <= Bytecode::Instruction::BinaryOperation::Mod);
					const auto idx = (int)isn.binOp - (int)Bytecode::Instruction::BinaryOperation::Div;

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
				if(uint32_t imm; ra.getTosImm(imm) && imm < 256)
				{
					ra.drop(a, 1);
					const auto n = ra.consume(a);

					a.emit(ArmV6::cmp(n, imm));
				}
				else
				{
					const auto m = ra.consume(a);
					const auto n = ra.consume(a);

					a.emit(ArmV6::cmp(n, m));
				}

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

				assert((size_t)isn.cond < sizeof(condLookup));

				const auto targetIdx = isn.jumpTargetIdx;

				harmonizeState(a, ra, labels[targetIdx]);
				a.emit(ArmV6::condBranch(condLookup[(size_t)isn.cond], Assembler::Label(targetIdx)));

				break;
			}
			case Bytecode::Instruction::OperationGroup::Jump:
			{
				const auto targetIdx = isn.jumpTargetIdx;

				harmonizeState(a, ra, labels[targetIdx]);
				a.emit(ArmV6::b(Assembler::Label(targetIdx)));

				break;
			}
			case Bytecode::Instruction::OperationGroup::Label:
			{
				const auto targetIdx = labelIdx++;

				harmonizeState(a, ra, labels[targetIdx]);
				a.pin(Assembler::Label(targetIdx));

				break;
			}
			case Bytecode::Instruction::OperationGroup::Pull:
			{
				ra.pull(a, isn.param);
				break;
			}
			case Bytecode::Instruction::OperationGroup::Shove:
			{
				ra.shove(a, isn.param);
				break;
			}
			case Bytecode::Instruction::OperationGroup::Drop:
			{
				ra.drop(a, isn.param);
				break;
			}
			case Bytecode::Instruction::OperationGroup::Call:
			{
				assert(false);
				// TODO do all lazy calculations here and write result in a PCS compatible manner.

//				const auto r = ArmV6::LoReg(0);	// Must be r0 (TODO move to r0 if not).
//				const auto s = ArmV6::LoReg(1);
//				a.emit(ArmV6::pop(ArmV6::LoRegs{}.add(r)));
//				a.emit(ArmV6::add(r, ArmV6::AnyReg(10)));
//				a.emit(ArmV6::ldr(s, r, ArmV6::Uoff<2, 5>(0)));
//				a.emit(ArmV6::blx(s));

//				assert(isn.call.nArgs <= stackDepth);
//
//				stackDepth += isn.call.nRet - isn.call.nArgs;

				break;
			}
			case Bytecode::Instruction::OperationGroup::Return:
			{
				arrangeReturn(a, ra, info.nRet);
				emitRetBranch = true;

				break;
			}
		}
	}

	assert(labelIdx == aRes.nLabels);

	if(!emitRetBranch)
	{
		arrangeReturn(a, ra, info.nRet);
	}

	a.pin(end);
	a.vmTab((VMTAB_LEAVE_NON_LEAF_INDEX << VMTAB_SHIFT) | info.nRet);

	return a.assemble();
}
