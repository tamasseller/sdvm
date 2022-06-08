#include "Program.h"

#include <stdexcept>

void Program::init(Visa::FrameInfo &fInfo)
{
	fIt = functions.cbegin();
	bIt = fIt->body.cbegin();
	iIt = bIt->cbegin();

	fInfo = fIt->info;
}

Program::RestorePoint Program::openFunction(uint32_t idx, Visa::FrameInfo &fInfo)
{
	if(functions.size() <= idx)
	{
		throw std::runtime_error("Unknown function called: " + std::to_string(idx) + " (should be below " + std::to_string(functions.size()));
	}

	RestorePoint ret
	{
		(uint32_t)(fIt - functions.cbegin()),
		(uint32_t)(bIt - fIt->body.cbegin()),
		(uint32_t)(iIt - bIt->cbegin())
	};

	fIt = functions.cbegin() + idx;
	bIt = fIt->body.cbegin();
	iIt = bIt->cbegin();

	fInfo = fIt->info;
	return ret;
}

void Program::restore(RestorePoint& rp, Visa::FrameInfo& fInfo)
{
	fIt = functions.cbegin() + rp.fIdx;
	bIt = fIt->body.cbegin() + rp.bIdx;
	iIt = bIt->cbegin() + rp.iIdx;

	fInfo = fIt->info;
}

void Program::seekBlock(uint32_t idx)
{
	if(fIt->body.size() <= idx)
	{
		throw std::runtime_error("Jump to unknown block: " + std::to_string(idx) + " (should be below " + std::to_string(fIt->body.size()));
	}

	bIt = fIt->body.cbegin() + idx;
	iIt = bIt->cbegin();
}

bool Program::readNext(Visa::Instruction& isn)
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
