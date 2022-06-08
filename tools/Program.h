#ifndef INMEMORYPROGRAMREADER_H_
#define INMEMORYPROGRAMREADER_H_

#include "vm/Visa.h"

#include <vector>

struct Program
{
	using Block = std::vector<Visa::Instruction>;
	using Body = std::vector<Block>;

	struct Function
	{
		Visa::FrameInfo info;
		Body body;
	};

	std::vector<Function> functions;
	decltype(functions.cbegin()) fIt;
	decltype(fIt->body.cbegin()) bIt;
	decltype(bIt->cbegin()) iIt;

	void init(Visa::FrameInfo &fInfo);

	struct RestorePoint
	{
		uint32_t fIdx, bIdx, iIdx;
	};

	RestorePoint openFunction(uint32_t idx, Visa::FrameInfo &fInfo);
	void restore(RestorePoint& rp, Visa::FrameInfo& fInfo);
	void seekBlock(uint32_t idx);
	bool readNext(Visa::Instruction& isn);
};

#endif /* INMEMORYPROGRAMREADER_H_ */
