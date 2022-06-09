#include "Program.h"

#include <stdexcept>

Program::Reader::Reader(const Program& p, Visa::FrameInfo &fInfo):
	p(p), fIt(p.functions.cbegin()), bIt(fIt->body.cbegin()), iIt(bIt->cbegin())
{
	fInfo = fIt->info;
}

Program::Reader::RestorePoint Program::Reader::openFunction(uint32_t idx, Visa::FrameInfo &fInfo)
{
	if(p.functions.size() <= idx)
	{
		throw std::runtime_error("Unknown function called: " + std::to_string(idx) + " (should be below " + std::to_string(p.functions.size()));
	}

	RestorePoint ret
	{
		(uint32_t)(fIt - p.functions.cbegin()),
		(uint32_t)(bIt - fIt->body.cbegin()),
		(uint32_t)(iIt - bIt->cbegin())
	};

	fIt = p.functions.cbegin() + idx;
	bIt = fIt->body.cbegin();
	iIt = bIt->cbegin();

	fInfo = fIt->info;
	return ret;
}

void Program::Reader::restore(RestorePoint& rp, Visa::FrameInfo& fInfo)
{
	fIt = p.functions.cbegin() + rp.fIdx;
	bIt = fIt->body.cbegin() + rp.bIdx;
	iIt = bIt->cbegin() + rp.iIdx;

	fInfo = fIt->info;
}

void Program::Reader::seekBlock(uint32_t idx)
{
	if(fIt->body.size() <= idx)
	{
		throw std::runtime_error("Jump to unknown block: " + std::to_string(idx) + " (should be below " + std::to_string(fIt->body.size()));
	}

	bIt = fIt->body.cbegin() + idx;
	iIt = bIt->cbegin();
}

bool Program::Reader::readNext(Visa::Instruction& isn)
{
	while(iIt == bIt->cend())
	{
		bIt++;

		if(bIt == fIt->body.cend())
		{
			return false;
		}

		iIt = bIt->cbegin();
	}

	isn = *iIt++;
	return true;
}
