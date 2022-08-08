#include "Analyzer.h"

#include "algorithm/Math.h"

static bool checkStackLevel(uint32_t &slot, uint32_t value)
{
	if(slot == 0)
	{
		slot = value + 1;
	}
	else if(slot != value + 1)
	{
		return false;
	}

	return true;
}

bool Analyzer::analyze(Analyzer::Result& ret, Bytecode::FunctionReader* reader)
{
	const auto fi = reader->reset();

	{
		int stackLevel = 0;
		int maxJumpTarget = -1u;

		ret.nLabels = 0;
		ret.maxStack = stackLevel = fi.nArgs;
		bool lastIsRet;

		for(Bytecode::Instruction isn; reader->read(isn);)
		{
			lastIsRet = false;

			switch(isn.g)
			{
			case Bytecode::Instruction::OperationGroup::Immediate:
				stackLevel++;
				break;

			case Bytecode::Instruction::OperationGroup::Binary:
				stackLevel--;
				break;

			case Bytecode::Instruction::OperationGroup::Conditional:
				stackLevel -= 2;
				// intentional no break;
			case Bytecode::Instruction::OperationGroup::Jump:
				maxJumpTarget = pet::max(maxJumpTarget, (int)isn.jumpTargetIdx);
				break;

			case Bytecode::Instruction::OperationGroup::Label:
				ret.nLabels++;
				break;

			case Bytecode::Instruction::OperationGroup::Pull:
				if(stackLevel <= isn.param)
				{
					return false;
				}

				stackLevel++;
				break;

			case Bytecode::Instruction::OperationGroup::Shove:
				if(!stackLevel || (stackLevel - 1) <= isn.param)
				{
					return false;
				}

				stackLevel--;
				break;

			case Bytecode::Instruction::OperationGroup::Drop:
				stackLevel -= isn.param;
				break;

			case Bytecode::Instruction::OperationGroup::Call:
				if(stackLevel < isn.nArgs)
				{
					return false;
				}

				stackLevel = stackLevel - isn.nArgs + isn.nRet;
				break;

			case Bytecode::Instruction::OperationGroup::Return:
				if(stackLevel < fi.nRet)
				{
					return false;
				}

				stackLevel -= fi.nRet;
				lastIsRet = true;
				break;
			}

			if(stackLevel < 0)
			{
				return false;
			}

			ret.maxStack = pet::max(ret.maxStack, (uint32_t)stackLevel);
		}

		if((int)ret.nLabels <= maxJumpTarget)
		{
			return false;
		}

		if(!lastIsRet && stackLevel < fi.nRet)
		{
			return false;
		}
	}

	if(ret.maxStack)
	{
		reader->reset();

		int stackLevel = fi.nArgs;
		uint32_t labelCounter = 0;
		uint32_t stackLevels[ret.maxStack] = {0,};

		for(Bytecode::Instruction isn; reader->read(isn);)
		{
			switch(isn.g)
			{
			case Bytecode::Instruction::OperationGroup::Immediate: stackLevel++; break;
			case Bytecode::Instruction::OperationGroup::Binary: stackLevel--; break;
			case Bytecode::Instruction::OperationGroup::Conditional:
				stackLevel -= 2;
				// intentional no break;
			case Bytecode::Instruction::OperationGroup::Jump:
				if(!checkStackLevel(stackLevels[isn.jumpTargetIdx], stackLevel))
				{
					return false;
				}

				break;

			case Bytecode::Instruction::OperationGroup::Label:
				if(!checkStackLevel(stackLevels[labelCounter], stackLevel))
				{
					return false;
				}

				labelCounter++;
				break;

			case Bytecode::Instruction::OperationGroup::Pull:
				stackLevel++;
				break;

			case Bytecode::Instruction::OperationGroup::Shove:
				stackLevel--;
				break;

			case Bytecode::Instruction::OperationGroup::Drop:
				stackLevel -= isn.nArgs;
				break;

			case Bytecode::Instruction::OperationGroup::Call: stackLevel = stackLevel - isn.nArgs + isn.nRet; break;
			case Bytecode::Instruction::OperationGroup::Return: stackLevel -= fi.nRet; break;
			}
		}
	}

	return true;
}
