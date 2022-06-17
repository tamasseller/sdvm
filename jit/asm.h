#ifndef ASM_H_
#define ASM_H_

#include "jit/armv6.h"

struct Assembler
{
	uint16_t *const startIsns, *nextIsn, *const endIsns;
	uint16_t **const labels;
	uint8_t const nLabels;

public:
	inline Assembler(uint16_t *firstIsn, uint16_t nIsns, uint16_t **firstLabel, uint8_t nLabels):
		startIsns(firstIsn), nextIsn(firstIsn), endIsns(firstIsn + nIsns), labels(firstLabel), nLabels(nLabels) {}

	struct Label
	{
		uint8_t idx;

		inline Label(uint8_t idx): idx(idx) {}

		template<size_t a, size_t n>
		operator ArmV6::Ioff<a, n> () const {
			return {(int16_t)(idx << a)};
		}
	};

	inline void emit(uint16_t isn)
	{
		assert(nextIsn < endIsns);
		*nextIsn++ = isn;
	}

	inline void label(const Label& l) const
	{
		assert(l.idx < nLabels);
		labels[l.idx] = nextIsn;
	}

	inline uint16_t *commit() const
	{
		for(uint16_t *p = startIsns; p != nextIsn; p++)
		{
			const auto isn = *p;
			uint16_t idx;

			if(ArmV6::getBranchIdxImm8(isn, idx))
			{
				assert(idx < nLabels);
				*p = ArmV6::setBranchIdxImm8(isn, (char*)labels[idx] - (char*)(p + 2));
			}
			else if(ArmV6::getBranchIdxImm11(isn, idx))
			{
				assert(idx < nLabels);
				*p = ArmV6::setBranchIdxImm11(isn, (char*)labels[idx] - (char*)(p + 2));
			}
		}

		return nextIsn;
	}
};

#endif /* ASM_H_ */
