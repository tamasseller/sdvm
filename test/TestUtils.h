#ifndef TESTUTILS_H_
#define TESTUTILS_H_

#include "Interpreter.h"
#include "Program.h"
#include "ExceptionalErrorHandler.h"

#include "1test/Test.h"

#include <sstream>

template<class Program, class ErrorHandler = ExceptionalErrorHandler>
static void doRunTest(const std::vector<uint32_t>& args, const std::vector<uint32_t>& exp, const Program &p)
{
	std::vector<uint32_t> result;

	try
	{
		auto r(p);
		result = Interpreter<ErrorHandler>::interpret(r, args);
	}
	catch(const std::exception& e)
	{
		FAIL(e.what());
	}

	CHECK(result.size() == exp.size());

	for(int i = 0; i < result.size(); i++)
	{
		if(result[i] != exp[i])
		{
			std::stringstream ss;
			ss << "Result mismatch at index " << i << ", expected " << exp[i] << " got " << result[i];
			const auto str = ss.str();
			FAIL(str.c_str());
		}
	}
}

#endif /* TESTUTILS_H_ */
