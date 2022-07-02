#include "Assembler.h"

void Assembler::emit(uint16_t isn)
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

void Assembler::vmTab(uint16_t isn)
{
	assert((void*)nextIsn < (void*)firstLiteral);	// GCOV_EXCL_LINE

	// Emit a marker udf that protects the inline data from being processed as an 
	// instruction in the post-processing steps, when the udf itself is going to 
	// be replaced with a "blx r9"
	*nextIsn++ = ArmV6::udf(0);

	assert((void*)nextIsn < (void*)firstLiteral);	// GCOV_EXCL_LINE
	*nextIsn++ = isn;
}

/// Calculate the jump offset for branch instructions.
inline intptr_t Assembler::getBranchOffset(uint16_t *pInstr, uint8_t idx) const
{
	assert(idx < nLabels);	// GCOV_EXCL_LINE

	// The offset needs to be calculated against the current instruction plus 4 bytes.
	return (char*)(startIsns + labels[idx].offset) - (char*)(pInstr + 2);
}

/// Calculate the PC-relative offset to be used to access a literal from an instruction.
inline uintptr_t Assembler::getLiteralOffset(uint16_t *pInstr, uint32_t *lit)
{
	assert(pInstr < (void*)lit); // GCOV_EXCL_LINE

	// The offset needs to be calculated against the word-aligned PC (which is the current instruction plus 4 bytes).
	return (uintptr_t)((char*)lit - (char*)(((uintptr_t)pInstr + 4) & ~3));
}

/*
 * The state before the assembly operation is as follows:
 *
 *  - All labels are pinned to their desired target location in the input instruction stream.
 *  - The end of the output area contains the registered literals (in reverse order).
 *  - The start of the output area is populated with the instruction stream with the following modifications:
 *     - the offset field of the literal access instructions contains the (reverse) index of the literal in
 *       the collection at the end of the output area;
 *     - conditional and unconditional branch instructions contain the index of the label they target in their
 *       offset field;
 *     - conditional branch instructions have a placeholder nop after them;
 *     - there are UDF instructions followed by an inline constant;
 *
 *    .-------+-------+-----+-------+---------------+-------+---------+-----+------.
 *   |  isn 0 | isn 1 | ... | isn N | <empty space> | lit N | lit N-1 | ... | lit 0 |
 *    '-------+-------+-----+-------+---------------+-------+---------+-----+------'
 *
 *  Processing is done in two linear scans over the instruction stream followed by copying of the literals to
 *  from the far end of the output area to right after the instructions.
 *
 *  The first scan compacts the instruction stream, consequently it may change the location of instructions.
 *  It may rewrite conditional branches as two instructions if the offset is deemed too high based on the
 *  preliminary value in the input. It also drops any loose nops it finds (which includes the placeholders of
 *  the untransformed conditional branches). It takes special care not to interpret the inline literals after
 *  the UDF instructions. Instructions are kept verbatim, changed or dropped but not added, so as to allow
 *  in-place operation. When an instruction is dropped, labels that point to later instructions are also updated.
 *
 *  The second scan does not move instructions around so label offsets and the location of the literal pool are
 *  stable at this point, so offset of branches and literal accesses can be fixed up. Branch targets are looked
 *  up from the label table and the correct PC offset is calculated and written in the place of the label index.
 *  Literal accesses are calculated based on the start address of the literal pool and the index in the offset
 *  field of the instruction, although the literals are not in their place just yet.
 *
 *  Depending on the parity of the length of the instruction stream, the literal pool may need an additional
 *  padding instruction. If so it is added before copying and reversing the literals, which can be done in a
 *  single go if the final place of the literal pool and the start of the collected literals (at the far end)
 *  does not overlap. If there is an overlap then the whole literal collection is first moved to the right
 *  place and then the order is flipped.
 */
uint16_t* Assembler::assemble()
{
	// Scan #1
	//
	// First go through the whole body and look for conditional branches and choose between the single instruction
	// direct approach (if the offset is small enough to fit in the 8bit immediate field) or a two instruction
	// sequence with a 11bit offset field width.
	//
	// As nops get removed the offsets of later jump targets get lower, which may bring those in the 8bit range,
	// however taking this into account would require multiple passes at its simplest. Also this phenomenon occurs
	// only if the 8bit offset boundary is just barely crossed, which is expected to be not that common in practice,
	// so this rare runtime performance optimization opportunity is traded for overall JIT efficiency.
	//
	uint16_t *o = startIsns;

	for(uint16_t *i = startIsns; i != nextIsn; i++)
	{
		const auto isn = *i;
		uint16_t idx;

		// Keep UDFs intact and skip inline literals associated with them.
		if(isn == ArmV6::udf(0))
		{
			assert(i < nextIsn); // GCOV_EXCL_LINE

			// Copy UDF.
			*o++ = isn;

			// Copy literal
			*o++ = *++i;

			// Skip regular copying.
			continue;
		}
		// Strip nops
		else if(isn == ArmV6::nop())
		{
			// Jump targets that are at higher addresses must be shifted down one instruction.
			for(auto n = 0u; n < nLabels; n++)
			{
				if((o - startIsns) < labels[n].offset)
				{
					labels[n].offset--;
				}
			}

			// Skip copying.
			continue;
		}
		// Check conditional branches if the offset fits in the 8bit immediate field.
		else if(ArmV6::getCondBranchOffset(isn, idx))
		{
			assert((i + 1) < (void*)firstLiteral && i[1] == ArmV6::nop());	// GCOV_EXCL_LINE
			assert(idx < 256);	// GCOV_EXCL_LINE

			if(const auto off = getBranchOffset(o, (uint8_t)idx); !ArmV6::Ioff<1, 8>::isInRange(off))
			{
				// Change original conditional to inverted condition and special target that will be filled in during the next operation to result in a skip-next behavior.
				const auto c = ArmV6::getBranchCondtion(isn);
				*o++ = ArmV6::condBranch(ArmV6::inverse(c), Label(skipNextSpecialLabelIdx));

				// Add unconditional branch to target (which has wider immediate field).
				*o++ = ArmV6::b(Label(idx));

				// The placeholder nop needs to consumed from the input stream in addition to the original branch.
				i++;

				// Skip copying.
				continue;
			}
		}

		// Every other instruction is copied verbatim (for now).
		*o++ = isn;
	}

	// Update address of last instruction as nops could have been removed.
	nextIsn = o;

	// Calculate word-aligned start address of literal pool.
	auto poolStart = (uint32_t *)(((uintptr_t)nextIsn + 3) & ~3);

	// Scan #2
	//
	// Fill in branch and literal offsets, replace UDFs.
	//
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
		// Replace udf with 'blx r9' and skip payload.
		else if(isn == ArmV6::udf(0))
		{
			*p = ArmV6::blx(ArmV6::AnyReg(9));
			assert(p < nextIsn); // GCOV_EXCL_LINE
			p++;
		}
	}

	const auto nLit = (uint32_t*)endIsns - firstLiteral;
	uint32_t * const poolEnd = poolStart + nLit;
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

	return (uint16_t*)nextIsn + (nLit << 1);
}
