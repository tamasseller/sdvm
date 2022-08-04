#ifndef ANALYZER_ANALYZER_H_
#define ANALYZER_ANALYZER_H_

#include "Bytecode.h"

struct Analyzer
{
	struct Result
	{
		uint32_t nLabels;
		uint32_t maxStack;
	};

	static bool analyze(Result& result, Bytecode::FunctionReader *);
};

#endif /* ANALYZER_ANALYZER_H_ */
