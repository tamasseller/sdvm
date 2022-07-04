#ifndef CODETESTUTILS_H_
#define CODETESTUTILS_H_

#include "1test/Test.h"

#include "jit/Bytecode.h"

#include <sstream>

class MockFunction
{
	std::vector<Bytecode::Instruction> body;

public:
	const Bytecode::FunctionInfo info;
	inline MockFunction(const Bytecode::FunctionInfo &info, const std::vector<Bytecode::Instruction> &body): body(body), info(info)	{}

	class Reader: public Bytecode::InstructionStreamReader
	{
		decltype(body)::const_iterator it, end;

		static inline bool read(Bytecode::InstructionStreamReader *isr, Bytecode::Instruction& isn)
		{
			auto self = static_cast<Reader*>(isr);

			if(self->it != self->end)
			{
				isn = *self->it++;
				return true;
			}

			return false;
		}
	public:
		inline Reader(const MockFunction& f): InstructionStreamReader(&read), it(f.body.cbegin()), end(f.body.cend()) {}
	};
};

static inline void checkCodeIs(const std::vector<std::string> &actual, const std::vector<std::string> &expected)
{
	auto ait = actual.begin();

	std::stringstream ss;

	int offs = 0;
	for(const auto &e: expected)
	{
		if(ait == actual.end())
		{
			ss << "Unexpected end of code, expected: " << e;
			const auto s = ss.str();
			FAIL(s.c_str());
		}

		if(*ait != e)
		{
			const auto offset = ait - actual.begin();

			ss << "Mismatching code at offset " << std::to_string(offset) << ":" << std::endl;

			const auto from = offset - std::min(offset, 3l);
			const auto len = std::max(expected.size(), actual.size());
			const auto to = std::min(offset + 4, (long)len);

			std::string actStr = "< actual >";
			int maxLen = actStr.length();
			for(int i = from; i < to; i++)
			{
				maxLen = std::max(maxLen, (i < actual.size()) ? (int)actual[i].size() : 0);
			}

			ss  << std::endl << "\t      " << actStr << std::string(maxLen - actStr.size() + 3, ' ') << "< expected >" << std::endl << std::endl;

			for(int i = from; i < to; i++)
			{
				const auto l = (i < actual.size()) ? actual[i] : std::string{};

				ss << "\t" << ((i == offset) ? "===>  " : "      ");
				ss << l << std::string(maxLen - l.size() + 3, ' ') << ((i < expected.size()) ? expected[i] : std::string{}) << std::endl;
			}

			const auto s = ss.str();
			FAIL(s.c_str());
		}

		ait++;
	}

	if(ait != actual.end())
	{
		ss << "Unexpected code at end:" << std::endl;

		for(int n = 5; n && ait != actual.end(); n--)
		{
			ss << "\t\t" << *ait++ << std::endl;
		}

		if(ait != actual.end())
		{
			ss << "\t\t..." << std::endl;
		}

		const auto s = ss.str();
		FAIL(s.c_str());
	}
}

#endif /* CODETESTUTILS_H_ */
