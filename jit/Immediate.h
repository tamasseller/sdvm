#ifndef JIT_IMMEDIATE_H_
#define JIT_IMMEDIATE_H_

#include <cstdint>

struct ImmediateFabricationPlan
{
	enum class LastOp: uint8_t {
		Add, Not, Shift, None
	};

	uint8_t imm, param;
	LastOp op;

	static inline bool make(uint32_t value, ImmediateFabricationPlan& out)
	{
		if(value < 256)
		{
			out.imm = value;
			out.op = LastOp::None;
		}
		else if(~value < 256)
		{
			out.imm = ~value;
			out.op = LastOp::Not;
		}
		else if(value < 511)
		{
			out = {.imm = (uint8_t)(value - 255), .param = 255, .op = LastOp::Add};
		}
		else
		{
			const auto shift = __builtin_ctz(value | 0x80000000u);
			const auto imm = value >> shift;

			if(imm < 256)
			{
				out = {.imm = (uint8_t)imm, .param = (uint8_t)shift, .op = LastOp::Shift};
			}
			else
			{
				return false;
			}
		}

		return true;
	}
};

#endif /* JIT_IMMEDIATE_H_ */
