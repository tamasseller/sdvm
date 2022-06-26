#ifndef JIT_IMMEDIATE_H_
#define JIT_IMMEDIATE_H_

#include <cstdint>

struct ImmediateFabricationPlan
{
	enum class LastOp: uint8_t {
		Add, Sub, Not, None
	};

	uint8_t imm, shift, param;
	LastOp op;

	static bool make(uint32_t value, ImmediateFabricationPlan& out);
};

#endif /* JIT_IMMEDIATE_H_ */
