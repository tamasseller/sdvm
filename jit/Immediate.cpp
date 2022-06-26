#include "Immediate.h"

bool attemptShift(uint32_t value, ImmediateFabricationPlan& out)
{
	const auto shift = __builtin_ctz(value | 0x80000000u);
	const auto imm = value >> shift;

	if(256 <= imm)
	{
		return false;
	}

	out.shift = shift;
	out.imm = imm;
	return true;
}

bool attemptShiftAndAdd(uint32_t value, ImmediateFabricationPlan& out)
{
	const auto offset = value & 0xff;

	if(attemptShift(value - offset, out))
	{
		out.param = offset;
		return true;
	}

	return false;
}

bool ImmediateFabricationPlan::make(uint32_t value, ImmediateFabricationPlan& out)
{
	if(value < 256)
	{
		out.imm = value;
		out.shift = 0;
		out.op = LastOp::None;
	}
	else if(~value < 256)
	{
		out.imm = ~value;
		out.shift = 0;
		out.op = LastOp::Not;
	}
	else if(value < 511)
	{
		out.imm = value - 255;
		out.shift = 0;
		out.param = 255;
		out.op = LastOp::Add;
	}
	else if(attemptShift(value, out))
	{
		out.op = LastOp::None;
	}
	else if(attemptShift(~value, out))
	{
		out.op = LastOp::Not;
	}
	else if(attemptShiftAndAdd(value, out))
	{
		out.op = LastOp::Add;
	}
	else if(attemptShiftAndAdd(value + 255, out))
	{
		out.param = 255 - out.param;
		out.op = LastOp::Sub;
	}
	else
	{
		return false;
	}

	return true;
}
