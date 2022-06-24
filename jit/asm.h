#ifndef ASM_H_
#define ASM_H_

#include "jit/armv6.h"

/**
 * Minimal function assembler for ARMv6-M code generation, it does:
 *
 *  - local jump target tracking and linking;
 *  - literal pool offset tracking and linking for a single literal pool;
 *  - automatic (transparent) conditional branch rewriting based on target
 *    offset width, a b<cond> <label> instructions may become:
 *    - a single instruction as-is for small offsets: b<cond> <offset> or
 *    - a two instruction sequence for longer jumps:
 *      1. b<!cond> <after-the-next> and then
 *      2. b <offset>.
 *
 *  Limitations:
 *   - max. local branch distance is +-2k
 *   - max. literal offset is 1k.
 */
struct Assembler
{
	static constexpr auto skipNextSpecialLabelIdx = uint8_t(-1u);

	// Instruction stream pointers.
	uint16_t *const startIsns, *nextIsn, *const endIsns;
	uint32_t *firstLiteral;

	// Jump target storage
	uint16_t **const labels;
	uint8_t const nLabels;

	/// Calculate the jump offset for branch instructions.
	inline intptr_t getBranchOffset(uint16_t *pInstr, uint8_t idx) const
	{
		assert(idx < nLabels);	// GCOV_EXCL_LINE

		// The offset needs to be calculated against the current instruction plus 4 bytes.
		return (char*)labels[idx] - (char*)(pInstr + 2);
	}

	/// TODO TBD
	inline uintptr_t getLiteralOffset(uint16_t *pInstr, uint32_t *lit) const
	{
		assert(pInstr < (void*)lit);

		// The offset needs to be calculated against the word-aligned PC (which is the current instruction plus 4 bytes).
		return (uintptr_t)((char*)lit - (char*)(((uintptr_t)pInstr + 4) & ~3));
	}

public:
	/**
	 * Initialize the assembler with instruction stream output space and storage for jump target (label) offsets.
	 *
	 * _firstIsn_ is the pointer to the start of the (output) code storage area and must be word-aligned.
	 * _nIsns_ is the number of 16-bit instructions that the output can hold, must be even due to alignment requirements
	 * _firstLabel_ is the pointer to the start of the label offset storage area.
	 * _nLabels_ is the number of 32-bit pointers that the label storage area can hold (used for space checking).
	 */
	inline Assembler(uint16_t *firstIsn, uint16_t nIsns, uint16_t **firstLabel, uint8_t nLabels):
		startIsns(firstIsn), nextIsn(startIsns), firstLiteral((uint32_t*)(startIsns + nIsns)), endIsns(startIsns + nIsns),
		labels(firstLabel), nLabels(nLabels)
	{
		// The highest label index is reserved for marking long conditional jump rewrites.
		assert(nLabels < skipNextSpecialLabelIdx); // GCOV_EXCL_LINE

		// The output stream starting point must be word aligned.
		assert(((uintptr_t)firstIsn & 3) == 0); // GCOV_EXCL_LINE

		// The output storage size must be word aligned.
		assert(((uintptr_t)nIsns & 1) == 0); // GCOV_EXCL_LINE
	}

	/**
	 * Helper used in place of the offset for emitting local branches.
	 */
	struct Label
	{
		// The label index.
		uint8_t idx;

		// Each label needs to have a unique index.
		inline Label(uint8_t idx): idx(idx) {}

		// Conversion that enables use for provisional inline encoding of branch targets.
		template<size_t a, size_t n>
		operator ArmV6::Ioff<a, n> () const {
			return {(int16_t)(idx << a)};
		}
	};

	/**
	 * Helper used in place of the offset for emitting PC-relative literal loads (LDR)
	 * and address generation (ADR). The indices stored in here are rewritten to actual
	 * PC-relative offsets once those are known.
	 */
	struct Literal
	{
		// The label index.
		uint8_t idx;

		// Each label needs to have a unique index.
		inline Literal(uint8_t idx): idx(idx) {}		// TODO make private

		// Conversion that enables use for provisional inline encoding of literal load targets.
		operator ArmV6::Uoff<2, 8> () const {
			return {(uint16_t)(idx << 2)};
		}
	};

	/**
	 * Emit an instruction.
	 *
	 * Conditional instructions take up two slots all other 16bit instructions take up one.
	 *
	 * Conditional branches must use an indexed _Label_ instance as jump target. All _Label_
	 * values need to be pinned to an actual location using the _label_ method.
	 */
	inline void emit(uint16_t isn)
	{
		assert((void*)nextIsn < (void*)firstLiteral);	// GCOV_EXCL_LINE

		*nextIsn++ = isn;

		if(ArmV6::isCondBranch(isn))
		{
			// Emit a placeholder nop in case the offset is too high for imm8 and the branch
			// needs to be rewritten, it gets removed later if not needed.
			*nextIsn++ = ArmV6::nop();
		}
	}

	/**
	 * Pin down a label after the previously emitted instruction (or before the first one if there is none).
	 *
	 * _Label_ instances that are used as a placeholder for local branch emission must be associated a
	 * jump target in the instruction stream using this method.
	 *
	 * The label index must be less than the size of the label storage specified at construction (_nLabels_).
	 */
	inline void label(const Label& l) const
	{
		assert(l.idx < nLabels);	// GCOV_EXCL_LINE
		labels[l.idx] = nextIsn;
	}

	/**
	 * Rewrite and link local branches and move literal pool and fill in offsets for loads (and address generation).
	 *
	 * Before calling this all labels referenced by emitted instruction must be pinned using the _label_ method.
	 * After calling this no more instructions should be emitted.
	 */
	inline void bodyDone()
	{
		// First go through the whole body and look for conditional branches and choose between the single instruction
		// direct approach (if the offset is small enough to fit in the 8bit immediate field) or a two instruction
		// sequence with a 11bit offset field width. The initially emitted extra nop gets removed if the short version
		// is used, which shrinks the length and consequently lowers the offsets of later jump targets, which may bring
		// those in the 8bit range, however taking this into account would require multiple passes at its simplest.
		// Also this phenomenon occurs only if the 8bit offset boundary is just barely crossed, which is expected to be
		// not that common, so this rare runtime performance optimization opportunity is traded for overall JIT efficiency.

		uint16_t *o = startIsns;

		for(uint16_t *i = startIsns; i != nextIsn; i++)
		{
			const auto isn = *i;
			uint16_t idx;

			// Check conditional branches if the offset fits in the 8bit immediate field.
			if(ArmV6::getCondBranchOffset(isn, idx))
			{
				assert((i + 1) < (void*)firstLiteral && i[1] == ArmV6::nop());	// GCOV_EXCL_LINE
				assert(idx < 256);	// GCOV_EXCL_LINE

				// There is another deliberately missed optimization opportunity here: if the offset is positive,
				// deleting the nop takes the target one instruction closer, so in the corner case when the offset
				// is one past the limit it could still be done using the short format with the original conditional
				// branch instruction. But again, it probably does not worth the effort to account for this.
				if(const auto off = getBranchOffset(o, (uint8_t)idx); ArmV6::Ioff<1, 8>::isInRange(off))
				{
					// Keep original instruction and get rid of placeholder nop if the offset fits.
					*o++ = isn;

					// Jump targets that are at higher addresses must be shifted down one instruction accordingly.
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

				// Either way, another instruction needs to consumed from the input stream in addition to the original branch.
				i++;
			}
			else
			{
				// Every other instruction is copied verbatim (for now).
				*o++ = isn;
			}
		}

		// Update address of last instruction as nops could have been removed.
		nextIsn = o;

		// Calculate word-aligned start address of literal pool.
		auto poolStart = (uint32_t *)(((uintptr_t)nextIsn + 3) & ~3);

		// Fill in branch and literal offsets
		for(uint16_t *p = startIsns; p != nextIsn; p++)
		{
			const auto isn = *p;
			uint16_t idx;

			// Update conditional branch targets using label offset from the table.
			if(ArmV6::getCondBranchOffset(isn, idx))
			{
				// Check if it is a re-written long conditional.
				if(idx == skipNextSpecialLabelIdx)
				{
					*p = ArmV6::setCondBranchOffset(isn, 0);
				}
				else
				{
					*p = ArmV6::setCondBranchOffset(isn, getBranchOffset(p, idx));
				}
			}
			// Update unconditional branch targets using label offset from the table.
			else if(ArmV6::getBranchOffset(isn, idx))
			{
				assert(idx != skipNextSpecialLabelIdx); // GCOV_EXCL_LINE
				*p = ArmV6::setBranchOffset(isn, getBranchOffset(p, idx));
			}
			// Update offset for literal load/address generation.
			else if(ArmV6::getLiteralOffset(isn, idx))
			{
				*p = ArmV6::setLiteralOffset(isn, getLiteralOffset(p, poolStart + idx));
			}
		}

		// Nop out padding between last instruction and literal pool if there is any.
		if(nextIsn != (void*)poolStart)
		{
			assert(nextIsn < (void*)firstLiteral); // GCOV_EXCL_LINE
			*nextIsn++ = ArmV6::nop();
		}

		assert(nextIsn == (void*)poolStart); // GCOV_EXCL_LINE

		// Move literals right after body and reverse their order while doing so in order to
		// get the ones added earlier slightly closer to the instructions that are referencing
		// them. This is a very slight improvement compared to directly copying in the same
		// order, however if there is enough room reversing is just as easy as not reversing,
		// and it is not too complicated even if the final location of the pool overlaps the
		// temporary site at the end of the output area (which is supposed to be rare anyway).
		uint32_t * const poolEnd = poolStart + ((uint32_t*)endIsns - firstLiteral);

		// Check if there is room (probable)
		if(poolEnd < firstLiteral)
		{
			const uint32_t *i = (const uint32_t*)endIsns;
			for(uint32_t *p = poolStart; p != poolEnd; p++)
			{
				*p = *--i;
			}
		}
		else
		{
			// If the final pool and the reversed collection at the end overlap, it first have
			// to be moved to the end of the body in order.
			const uint32_t *i = (const uint32_t*)firstLiteral;
			for(uint32_t *p = poolStart; p != poolEnd; p++)
			{
				*p = *i++;
			}

			// Then the order needs to be reversed in place.
			for(uint32_t *a = poolStart, *b = poolEnd - 1; a < b; a++, b--)
			{
				auto t = *a;
				*a = *b;
				*b = t;
			}
		}

	}

	/**
	 * Collect literals at the very end of the output area, return index for future reference.
	 *
	 * It looks for a duplicate before adding a new literal to eliminate useless redundancy.
	 */
	inline Literal literal(uint32_t v)
	{
		assert(((uintptr_t)firstLiteral & 3) == 0); // GCOV_EXCL_LINE

		for(auto l = firstLiteral; l != (void*)endIsns; l++)
		{
			if(*l == v)
			{
				return (uint32_t*)endIsns - l - 1;
			}
		}

		assert((void*)nextIsn < firstLiteral - 1); // GCOV_EXCL_LINE

		*--firstLiteral = v;
		return (uint32_t*)endIsns - firstLiteral - 1;
	}
};

#endif /* ASM_H_ */
