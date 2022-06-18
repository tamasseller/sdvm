#ifndef ASM_H_
#define ASM_H_

#include "jit/armv6.h"

struct Assembler
{
	static constexpr auto skipNextSpecialLabelIdx = uint8_t(-1u);

	uint16_t *const startIsns, *nextIsn, *const endIsns;
	uint16_t **const labels;
	uint8_t const nLabels;

	inline intptr_t getOffset(uint16_t *p, uint8_t idx) const
	{
		assert(idx < nLabels);	// GCOV_EXCL_LINE
		return (char*)labels[idx] - (char*)(p + 2);
	}

public:
	inline Assembler(uint16_t *firstIsn, uint16_t nIsns, uint16_t **firstLabel, uint8_t nLabels):
		startIsns(firstIsn), nextIsn(firstIsn), endIsns(firstIsn + nIsns), labels(firstLabel), nLabels(nLabels)
	{
		// The highest label index is reserved for marking long conditional jump rewrites.
		assert(nLabels < skipNextSpecialLabelIdx); // GCOV_EXCL_LINE
	}

	struct Label
	{
		uint8_t idx;

		inline Label(uint8_t idx): idx(idx) {}

		// Used for provisional inline encoding of branch targets.
		template<size_t a, size_t n>
		operator ArmV6::Ioff<a, n> () const {
			return {(int16_t)(idx << a)};
		}
	};

	inline void emit(uint16_t isn)
	{
		assert(nextIsn < endIsns);	// GCOV_EXCL_LINE

		*nextIsn++ = isn;

		if(ArmV6::isCondBranch(isn))
		{
			// Placeholder in case the offset is too high for imm8 (removed later if not needed).
			*nextIsn++ = ArmV6::nop();
		}
	}

	inline void label(const Label& l) const
	{
		assert(l.idx < nLabels);	// GCOV_EXCL_LINE
		labels[l.idx] = nextIsn;
	}

	inline uint16_t *commit()
	{
		// First choose branch types based offset width.
		uint16_t *o = startIsns;
		for(uint16_t *i = startIsns; i != nextIsn; i++)
		{
			const auto isn = *i;
			uint16_t idx;

			// Check conditional branches whether offset fits in immediate field
			if(ArmV6::getCondBranchOffset(isn, idx))
			{
				assert((i + 1) < endIsns && i[1] == ArmV6::nop());	// GCOV_EXCL_LINE
				assert(idx < 256);	// GCOV_EXCL_LINE

				// There is a deliberately missed optimization opportunity here: if the offset
				// is positive, deleting the nop takes the target one instruction closer, so in
				// the corner case when the offset is one past the limit it could still be done
				// using the short format with the original conditional branch instruction.
				// However it probably does not worth the effort to account for this.
				if(const auto off = getOffset(o, (uint8_t)idx); ArmV6::Ioff<1, 8>::isInRange(off))
				{
					// Keep original instruction and get rid of placeholder nop if the offset fits.
					*o++ = isn;

					// Jump targets that are at higher addresses must be updated accordingly.
					for(auto n = 0u; n < nLabels; n++)
					{
						if(i < labels[n])
						{
							labels[n]--;
						}
					}
				}
				else
				{
					// Change original conditional to inverted condition and special target that will be filled in during the next operation to result in a skip-next behavior.
					const auto c = ArmV6::getBranchCondtion(isn);
					*o++ = ArmV6::condBranch(ArmV6::inverse(c), Label(skipNextSpecialLabelIdx));

					// Add unconditional branch to target (which has wider immediate field).
					*o++ = ArmV6::b(Label(idx));
				}

				// Either
				i++;
			}
			else
			{
				*o++ = isn;
			}
		}

		// Update address of last instruction as nops could have been removed.
		nextIsn = o;

		// Fill in branch offsets
		for(uint16_t *p = startIsns; p != nextIsn; p++)
		{
			const auto isn = *p;
			uint16_t idx;

			// Update conditional branch targets via lookup
			if(ArmV6::getCondBranchOffset(isn, idx))
			{
				// Check if it is a re-written long conditional.
				if(idx == skipNextSpecialLabelIdx)
				{
					*p = ArmV6::setCondBranchOffset(isn, 0);
				}
				else
				{
					*p = ArmV6::setCondBranchOffset(isn, getOffset(p, idx));
				}
			}
			// Update unconditional branch targets via lookup
			else if(ArmV6::getBranchOffset(isn, idx))
			{
				assert(idx != skipNextSpecialLabelIdx); // GCOV_EXCL_LINE
				*p = ArmV6::setBranchOffset(isn, getOffset(p, idx));
			}
		}

		return nextIsn;
	}
};

#endif /* ASM_H_ */
