#include "RegisterAllocator.h"

#include "Assembler.h"

uint32_t RegisterAllocator::memoryOffset(uint16_t idx) const {
	return uint8_t(nRegArgs - idx - 1) << 2;
}

ArmV6::LoReg RegisterAllocator::correspondingRegister(uint16_t idx) {
	return ArmV6::LoReg((idx + 1) & 7);
}

void RegisterAllocator::summonImmediate(Assembler& a, ArmV6::LoReg target, uint32_t value)
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

void RegisterAllocator::loadCopy(Assembler& a, ArmV6::LoReg reg, uint16_t idx)
{
	assert(idx < stackDepth && stackDepth <= idx + 8); // GCOV_EXCL_LINE
	auto &status = topEight[idx & 7].valueStatus;

	if(status == ValueStatus::Unloaded)
	{
		a.emit(ArmV6::ldrSp(correspondingRegister(idx), memoryOffset(idx)));
		status = ValueStatus::Clean;
	}

	assert(status == ValueStatus::Clean || status == ValueStatus::Dirty); // GCOV_EXCL_LINE
	a.emit(ArmV6::mov(reg, correspondingRegister(idx)));
}

void RegisterAllocator::store(Assembler& a, uint16_t idx)
{
	eliminateCopies(a, idx);

	const auto off = memoryOffset(idx);
	a.emit(ArmV6::strSp(correspondingRegister(idx), off));
	maxStored = pet::max(maxStored, idx);
}

void RegisterAllocator::eliminateCopies(Assembler &a, uint16_t idx)
{
	uint32_t replacementIdx;
	bool replacementFound = false;

	for(auto i = pet::max(stackDepth, uint16_t(8)) - 8u; i <= stackDepth; i++)
	{
		auto& placement = topEight[i & 7];

		if(i != idx && placement.valueStatus == ValueStatus::Copy && placement.deferredParam == idx)
		{
			if(!replacementFound)
			{
				replacementFound = true;
				replacementIdx = i;
				loadCopy(a, correspondingRegister(i), idx);
				placement.valueStatus = ValueStatus::Dirty;
			}
			else
			{
				placement.deferredParam = replacementIdx;
			}
		}
	}
}

RegisterAllocator::RegisterAllocator(uint32_t nArgs): nArgs(nArgs), stackDepth(nArgs), nRegArgs(pet::min(nArgs, uint32_t(3u)))
{
	for(auto i = 0u; i < nRegArgs; i++)
	{
		topEight[i].valueStatus = ValueStatus::Dirty;
	}

	const auto k = pet::min(nArgs, uint32_t(8u));
	for(auto i = nRegArgs; i < k; i++)
	{
		topEight[i].valueStatus = ValueStatus::Unloaded;
	}

	for(auto i = k; i < 8; i++)
	{
		topEight[i].valueStatus = ValueStatus::Empty;
	}
}

ArmV6::LoReg RegisterAllocator::deactivate(Assembler &a, uint16_t idx, const ValuePlacement& newPlacement)
{
	assert(idx < stackDepth && stackDepth <= idx + 8); // GCOV_EXCL_LINE

	const auto reg = correspondingRegister(idx);
	auto &placement = topEight[idx & 7];

	switch(placement.valueStatus)
	{
		case ValueStatus::Empty: assert(false); // GCOV_EXCL_LINE
		case ValueStatus::Unloaded:
		case ValueStatus::Clean:
			break;
		case ValueStatus::Dirty:
			store(a, idx);
			break;
		case ValueStatus::Copy:
			loadCopy(a, reg, placement.deferredParam);
			store(a, idx);
			break;

		case ValueStatus::Immediate:
			summonImmediate(a, reg, placement.deferredParam);
			store(a, idx);
			break;
	}

	placement = newPlacement;
	return reg;
}

ArmV6::LoReg RegisterAllocator::allocate(Assembler& a, ValueStatus status, uint32_t param)
{
	ArmV6::LoReg reg;

	if(7 < stackDepth)
	{
		reg = deactivate(a, stackDepth - 8, {status, param});
	}
	else
	{
		assert(topEight[stackDepth & 7].valueStatus == ValueStatus::Empty);
		topEight[stackDepth & 7] = {status, param};
		reg = correspondingRegister(stackDepth);
	}

	stackDepth++;
	return reg;
}

ArmV6::LoReg RegisterAllocator::replace(Assembler& a)
{
	assert(0 < stackDepth); // GCOV_EXCL_LINE

	const auto idx = stackDepth - 1;
	const auto topSlotIdx = idx & 7;
	const auto reg = correspondingRegister(idx);

	switch(topEight[topSlotIdx].valueStatus)
	{
	case ValueStatus::Empty: assert(false);	// GCOV_EXCL_LINE
		break;
	case ValueStatus::Unloaded:
		a.emit(ArmV6::ldrSp(correspondingRegister(idx), memoryOffset(idx)));
		// no break
	case ValueStatus::Clean:
	case ValueStatus::Dirty:
		eliminateCopies(a, idx);
		break;

	case ValueStatus::Copy:
		loadCopy(a, reg, topEight[topSlotIdx].deferredParam);
		break;

	case ValueStatus::Immediate:
		summonImmediate(a, reg, topEight[topSlotIdx].deferredParam);
		break;
	}

	topEight[topSlotIdx].valueStatus = ValueStatus::Dirty;

	return reg;
}


ArmV6::LoReg RegisterAllocator::consume(Assembler& a)
{
	assert(0 < stackDepth); // GCOV_EXCL_LINE

	const auto idx = stackDepth - 1;
	const auto topSlotIdx = idx & 7;
	auto reg = correspondingRegister(idx);

	switch(topEight[topSlotIdx].valueStatus)
	{
	case ValueStatus::Empty: assert(false);	// GCOV_EXCL_LINE
		break;
	case ValueStatus::Unloaded:
		a.emit(ArmV6::ldrSp(correspondingRegister(idx), memoryOffset(idx)));
		// no break
	case ValueStatus::Clean:
	case ValueStatus::Dirty:
		eliminateCopies(a, idx);
		break;

	case ValueStatus::Copy:
		{
			const auto target = topEight[topSlotIdx].deferredParam;
			reg = correspondingRegister(target);

			assert(target < stackDepth && stackDepth <= target + 8); // GCOV_EXCL_LINE

			if(auto &status = topEight[target & 7].valueStatus; status == ValueStatus::Unloaded)
			{
				a.emit(ArmV6::ldrSp(reg, memoryOffset(target)));
				status = ValueStatus::Clean;
			}
		}
		break;

	case ValueStatus::Immediate:
		summonImmediate(a, reg, topEight[topSlotIdx].deferredParam);
		break;
	}

	stackDepth = idx;
	topEight[topSlotIdx].valueStatus = (7 < idx) ? ValueStatus::Unloaded : ValueStatus::Empty;

	return reg;
}

void RegisterAllocator::pull(Assembler& a, uint32_t idx)
{
	assert(idx < stackDepth); // GCOV_EXCL_LINE

	if(stackDepth < idx + 8)
	{
		auto & placement = topEight[idx & 7];

		if(placement.valueStatus == ValueStatus::Copy || placement.valueStatus == ValueStatus::Immediate)
		{
			allocate(a, placement.valueStatus, placement.deferredParam);
		}
		else
		{
			assert(placement.valueStatus != ValueStatus::Empty); // GCOV_EXCL_LINE
			allocate(a, ValueStatus::Copy, idx);
		}
	}
	else
	{
		a.emit(ArmV6::ldrSp(acquire(a), memoryOffset(idx)));
	}
}

void RegisterAllocator::shove(Assembler& a, uint32_t dstIdx)
{
	assert(dstIdx < stackDepth - 1); // GCOV_EXCL_LINE

	if(stackDepth <= dstIdx + 8)
	{
		const auto srcIdx = stackDepth - 1;
		auto &srcPlacement = topEight[srcIdx & 7];
		auto &dstPlacement = topEight[dstIdx & 7];

		if(srcPlacement.valueStatus != ValueStatus::Copy || srcPlacement.deferredParam != dstIdx)
		{
			eliminateCopies(a, dstIdx);

			if(srcPlacement.valueStatus == ValueStatus::Copy || srcPlacement.valueStatus == ValueStatus::Immediate)
			{
				dstPlacement = srcPlacement;
			}
			else
			{
				dstPlacement.valueStatus = ValueStatus::Dirty;

				if(isInReg(srcPlacement.valueStatus))
				{
					a.emit(ArmV6::mov(correspondingRegister(dstIdx), correspondingRegister(srcIdx)));
				}
				else
				{
					assert(srcPlacement.valueStatus == ValueStatus::Unloaded); // GCOV_EXCL_LINE
					a.emit(ArmV6::ldrSp(correspondingRegister(dstIdx), memoryOffset(srcIdx)));
				}
			}

			srcPlacement.valueStatus = (7 < stackDepth) ? ValueStatus::Unloaded : ValueStatus::Empty;
		}
		else
		{
			srcPlacement.valueStatus = (7 < stackDepth) ? ValueStatus::Unloaded : ValueStatus::Empty;
		}

		--stackDepth;
	}
	else
	{
		a.emit(ArmV6::strSp(consume(a), memoryOffset(dstIdx)));
	}
}

void RegisterAllocator::drop(Assembler& a, uint32_t n)
{
	assert(n <= stackDepth); // GCOV_EXCL_LINE

	while(n--)
	{
		eliminateCopies(a, stackDepth - 1);
		topEight[(stackDepth - 1) & 7].valueStatus = (7 < stackDepth) ? ValueStatus::Unloaded : ValueStatus::Empty;
		stackDepth--;
	}
}

void RegisterAllocator::flushDeferred(Assembler &a)
{
	for(auto i = pet::max(stackDepth, uint16_t(8)) - 8u; i <= stackDepth; i++)
	{
		const auto slotIdx = i & 7;
		auto &placement = topEight[slotIdx];

		if(placement.valueStatus == ValueStatus::Copy)
		{
			loadCopy(a, correspondingRegister(i), placement.deferredParam);
			placement.valueStatus = ValueStatus::Dirty;
		}
		else if(placement.valueStatus == ValueStatus::Immediate)
		{
			summonImmediate(a, correspondingRegister(i), placement.deferredParam);
			placement.valueStatus = ValueStatus::Dirty;
		}
	}
}

RegisterAllocator::Signature RegisterAllocator::getState()
{
	uint16_t loadedness;

	for(auto& p: topEight)
	{
		assert((int)p.valueStatus < 4);
		loadedness = (loadedness << 2) | (uint16_t)p.valueStatus;
	}

	return Signature
	{
		.loadedness = loadedness,
		.stackDepth = stackDepth // TODO remove after validator is implemented
	};
}

inline bool RegisterAllocator::isInReg(ValueStatus v)
{
	if(v == ValueStatus::Unloaded)
	{
		return false;
	}

	assert(v == ValueStatus::Clean || v == ValueStatus::Dirty);
	return true;
}

void RegisterAllocator::applyState(Assembler &a, Signature sgn)
{
	assert(sgn.stackDepth == stackDepth); // TODO remove after validator is implemented
	auto loadedness = sgn.loadedness;

	for(int i = 0; i < 8; i++)
	{
		const auto required = ValueStatus(loadedness >> 14);
		const auto idx = (stackDepth & ~7) + i;
		auto &current = topEight[i].valueStatus;

		assert((current == ValueStatus::Empty) == (required == ValueStatus::Empty));

		if(current != ValueStatus::Empty)
		{
			if(isInReg(current) && !isInReg(required))
			{
				store(a, idx);
			}
			else if(!isInReg(current) && isInReg(required))
			{
				a.emit(ArmV6::ldrSp(correspondingRegister(idx), memoryOffset(idx)));
			}
		}

		current = required;
		loadedness <<= 2;
	}
}
