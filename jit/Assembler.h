#ifndef ASSEMBLER_H_
#define ASSEMBLER_H_

#include "jit/armv6.h"

/**
 * Mostly in-place assembler for ARMv6-M code generation, it does:
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
class Assembler
{
public:
	struct LabelInfo
	{
		uint16_t offset;
	};

private:
	static constexpr auto skipNextSpecialLabelIdx = uint8_t(-1u);

	// Instruction stream pointers.
	uint16_t *const startIsns, *nextIsn, *const endIsns;
	uint32_t *firstLiteral;

	// Jump target storage
	LabelInfo *const labels;
	uint8_t const nLabels;

	/// Calculate the jump offset for branch instructions.
	inline intptr_t getBranchOffset(uint16_t *pInstr, uint8_t idx) const
	{
		assert(idx < nLabels);	// GCOV_EXCL_LINE

		// The offset needs to be calculated against the current instruction plus 4 bytes.
		return (char*)(startIsns + labels[idx].offset) - (char*)(pInstr + 2);
	}

	/// Calculate the PC-relative offset to be used to access a literal from an instruction.
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
	inline Assembler(uint16_t *firstIsn, uint16_t nIsns, LabelInfo *firstLabel, uint8_t nLabels):
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
	class Label
	{
		friend Assembler;

		// The label index.
		uint8_t idx;

	public:
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
	class Literal
	{
		friend Assembler;

		// The label index.
		uint8_t idx;

		// Each label needs to have a unique index.
		inline Literal(uint8_t idx): idx(idx) {}		// TODO make private
	public:

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
	 * values need to be pinned to an actual location using the _pin_ method before final
	 * assembly.
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
	inline void pin(const Label& l) const
	{
		assert(l.idx < nLabels);	// GCOV_EXCL_LINE
		labels[l.idx].offset = nextIsn - startIsns;
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

	/**
	 * Rewrite and link local branches and move literal pool and fill in offsets for loads (and address generation).
	 *
	 * Before calling this all labels referenced by emitted instruction must be pinned using the _pin_ method.
	 * After calling this no more instructions should be emitted.
	 */
	uint16_t* assemble();
};

#endif /* ASSEMBLER_H_ */
