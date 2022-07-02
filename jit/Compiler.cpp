#include "Compiler.h"

#include "VmTab.h"
#include "Immediate.h"
#include "Assembler.h"

#include "algorithm/Math.h"

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

struct StackState
{
	/*
	 *                    Newly calculated
	 *                            |
	 *  In-memory argument        |           Register argument
	 *         |                  |                  |
	 *         V                  V                  V
	 *    .--------.   load    .-----.   shove    .-----.
	 *   | Unloaded | ------> | Clean | -------> | Dirty |
	 *    '--------'           '-----'  <-------  '-----'
	 *                                   spill
	 *
	 *
	 */
	enum class ValueStatus
	{
		Empty,		// There is no value associated with this slot.
		Unloaded, 	// The value resides in the associated memory location.
		Clean,    	// The value is loaded into the corresponding register.
		Dirty,    	// The corresponding register contains a new value that is not yet written in memory.
	};

	struct ValuePlacement
	{
		ValueStatus status;
	};

	ValuePlacement topEight[8];
	const uint32_t nArgs;
	uint32_t stackDepth;
	uint32_t maxStack = 0;
	uint8_t usedCsRegs = 0;


	StackState(uint32_t nArgs): nArgs(nArgs), stackDepth(nArgs)
	{
		for(auto i = 0u; i < pet::min(nArgs, uint32_t(4u)); i++)
		{
			topEight[i].status = ValueStatus::Dirty;
		}

		for(auto i = nArgs; i < 8u; i++)
		{
			topEight[i].status = ValueStatus::Empty;
		}
	}

	ArmV6::LoReg allocate(Assembler& a)
	{
		const auto idx = stackDepth++;
		const auto ret = idx  % 8;
		maxStack = pet::max(maxStack, stackDepth);

		switch(topEight[ret].status)
		{
		case ValueStatus::Empty:
			usedCsRegs |= (1 << idx);
			break;
		case ValueStatus::Unloaded:
		case ValueStatus::Clean:
			break;
		case ValueStatus::Dirty:
			a.emit(ArmV6::strSp(ArmV6::LoReg(ret), (nArgs - idx) << 2));
			break;
		}

		topEight[ret].status = ValueStatus::Dirty;

		return ArmV6::LoReg(ret);
	}

	ArmV6::LoReg consume(Assembler& a)
	{
		assert(0 < stackDepth);

		const auto idx = --stackDepth;
		const auto ret = idx  % 8;

		switch(topEight[ret].status)
		{
		case ValueStatus::Empty:
			assert(false);
			break;
		case ValueStatus::Unloaded:
			a.emit(ArmV6::ldrSp(ArmV6::LoReg(ret), (nArgs - idx) << 2));
			break;
		case ValueStatus::Clean:
		case ValueStatus::Dirty:
			break;
		}

		topEight[ret].status = (7 < idx) ? ValueStatus::Unloaded : ValueStatus::Empty;

		return ArmV6::LoReg(ret);
	}

	ArmV6::LoReg replace(Assembler& a)
	{
		assert(0 < stackDepth);

		const auto idx = stackDepth - 1;
		const auto ret = idx  % 8;

		switch(topEight[ret].status)
		{
		case ValueStatus::Empty:
			assert(false);
			break;
		case ValueStatus::Unloaded:
			a.emit(ArmV6::ldrSp(ArmV6::LoReg(ret), (nArgs - idx) << 2));
			break;
		case ValueStatus::Clean:
		case ValueStatus::Dirty:
			break;
		}

		topEight[ret].status = ValueStatus::Dirty;

		return ArmV6::LoReg(ret);
	}

	void pull(Assembler& a, ArmV6::LoReg reg, uint32_t idx)
	{
		assert(idx < stackDepth);

		if(stackDepth < idx + 8)
		{
			const auto src = idx % 8;
			switch(topEight[src].status)
			{
			case ValueStatus::Empty:
				assert(false);
				break;
			case ValueStatus::Unloaded:
				break;
			case ValueStatus::Clean:
			case ValueStatus::Dirty:
				a.emit(ArmV6::mov(reg, ArmV6::LoReg(src)));
				return;
			}
		}

		a.emit(ArmV6::ldrSp(reg, (nArgs - idx) << 2));
	}

	void shove(Assembler& a, ArmV6::LoReg reg, uint32_t idx)
	{
		assert(idx < stackDepth);

		if(stackDepth < idx + 8)
		{
			const auto dst = idx % 8;
			topEight[dst].status = ValueStatus::Dirty;
			a.emit(ArmV6::mov(ArmV6::LoReg(dst), reg));
			return;
		}

		a.emit(ArmV6::strSp(reg, (nArgs - idx) << 2));
	}

	void drop(uint32_t n)
	{
		assert(n <= stackDepth);

		while(n--)
		{
			const auto idx = --stackDepth;
			const auto ret = idx % 8;
			topEight[ret].status = (7 < idx) ? ValueStatus::Unloaded : ValueStatus::Empty;
		}
	}
};

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

	StackState ss(info.nArgs);

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
				summonImmediate(a, ss.allocate(a), isn.imm.value);
				break;
			}

			case Bytecode::Instruction::OperationGroup::Binary:
			{
				const auto m = ss.consume(a);
				const auto nd = ss.replace(a);

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
				const auto m = ss.consume(a);
				const auto n = ss.consume(a);

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
						ss.pull(a, ss.allocate(a), isn.move.param);
						break;
					}
					case Bytecode::Instruction::MoveOperation::Shove:
					{
						ss.shove(a, ss.consume(a), isn.move.param);
						break;
					}
					case Bytecode::Instruction::MoveOperation::Drop:
					{
						ss.drop(isn.move.param);
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
