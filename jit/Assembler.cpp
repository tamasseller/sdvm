#include "Assembler.h"

uint16_t* Assembler::assemble()
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
					if((i - startIsns) < labels[n].offset)
					{
						labels[n].offset--;
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

	uint32_t * const poolEnd = poolStart + ((uint32_t*)endIsns - firstLiteral);
	if(poolEnd != poolStart)
	{
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

	return (uint16_t*)poolEnd;
}
