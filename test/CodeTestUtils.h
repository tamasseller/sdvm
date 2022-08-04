#ifndef CODETESTUTILS_H_
#define CODETESTUTILS_H_

#include "1test/Test.h"

#include "Bytecode.h"

#include <sstream>
#include <vector>

class MockFunction
{
	std::vector<Bytecode::Instruction> body;

public:
	const Bytecode::FunctionInfo info;
	inline MockFunction(const Bytecode::FunctionInfo &info, const std::vector<Bytecode::Instruction> &body): body(body), info(info)	{}

	class Reader: public Bytecode::FunctionReader
	{
		const Bytecode::FunctionInfo info;
		decltype(body)::const_iterator begin, it, end;

		static inline uint64_t read(Bytecode::FunctionReader *isr)
		{
			auto self = static_cast<Reader*>(isr);

			if(self->it != self->end)
			{
				return *self->it++;
			}

			Bytecode::Instruction ret;
			ret.g = Bytecode::Instruction::OperationGroup::Invalid;
			return ret;
		}

		static inline Bytecode::FunctionInfo reset(Bytecode::FunctionReader *isr)
		{
			auto self = static_cast<Reader*>(isr);

			self->it = self->begin;

			return self->info;
		}
	public:
		inline Reader(const MockFunction& f): FunctionReader(&read, &reset), info(f.info), begin(f.body.cbegin()), end(f.body.cend()) {}
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

namespace TestCode {

inline const MockFunction square
{
	Bytecode::FunctionInfo{.nArgs = 1, .nRet = 1},
	{
		Bytecode::pull(0),
		Bytecode::mul()
	}
};

inline const MockFunction addThree
{
	Bytecode::FunctionInfo{.nArgs = 1, .nRet = 1},
	{
		Bytecode::immediate(3),
		Bytecode::add(),
		Bytecode::ret()
	}
};

inline const MockFunction add123
{
	Bytecode::FunctionInfo{.nArgs = 1, .nRet = 1},
	{
		Bytecode::immediate(123),
		Bytecode::add(),
		Bytecode::ret()
	}
};

inline const MockFunction add3reverse
{
	Bytecode::FunctionInfo{.nArgs = 1, .nRet = 1},
	{
		Bytecode::immediate(3),
		Bytecode::pull(0),
		Bytecode::add(),
		Bytecode::ret()
	}
};

inline const MockFunction subThree
{
	Bytecode::FunctionInfo{.nArgs = 1, .nRet = 1},
	{
		Bytecode::immediate(3),
		Bytecode::sub(),
		Bytecode::ret()
	}
};

inline const MockFunction sub123
{
	Bytecode::FunctionInfo{.nArgs = 1, .nRet = 1},
	{
		Bytecode::immediate(123),
		Bytecode::sub(),
		Bytecode::ret()
	}
};

inline const MockFunction add1024Times400GreaterThanNegative100
{
	Bytecode::FunctionInfo{.nArgs = 1, .nRet = 1},
	{
		Bytecode::immediate(1024),
		Bytecode::add(),
		Bytecode::immediate(400),
		Bytecode::mul(),
		Bytecode::immediate(-100),
		Bytecode::jsgt(0),
			Bytecode::immediate(0),
			Bytecode::ret(),
		Bytecode::label(),
			Bytecode::immediate(1),
	}
};

inline const MockFunction setLowByte
{
	Bytecode::FunctionInfo{.nArgs = 2, .nRet = 1},
	{
		Bytecode::pull(0),
		Bytecode::immediate(8),
		Bytecode::rsh(),
		Bytecode::immediate(8),
		Bytecode::lsh(),

		Bytecode::pull(1),
		Bytecode::immediate(0xff),
		Bytecode::aAnd(),

		Bytecode::aOr(),
		Bytecode::ret()
	}
};

inline const MockFunction subXorAsh
{
	Bytecode::FunctionInfo{.nArgs = 4, .nRet = 1},
	{
		Bytecode::sub(),
		Bytecode::aXor(),
		Bytecode::ash(),
	}
};

inline const MockFunction consumeFnv
{
	Bytecode::FunctionInfo{.nArgs = 2, .nRet = 1},
	{
		Bytecode::aXor(),
		Bytecode::immediate(16777619),
		Bytecode::mul(),
		Bytecode::ret()
	}
};

inline const MockFunction popCount
{
	Bytecode::FunctionInfo{.nArgs = 1, .nRet = 1},
	{
		Bytecode::immediate(0),	// int ret = 0;

		Bytecode::immediate(0), // int i = 0;

		Bytecode::label(),		// do {

		Bytecode::immediate(1), //     const auto m = (1 << i);
		Bytecode::pull(2),
		Bytecode::lsh(),

		Bytecode::pull(0),      //     const auto n = m & arg;
		Bytecode::aAnd(),

		Bytecode::immediate(0),
		Bytecode::jeq(1),		//     if(n != 0) {
		Bytecode::pull(1),		//         ret++;
		Bytecode::immediate(1),
		Bytecode::add(),
		Bytecode::shove(1),

		Bytecode::label(),		//     }

		Bytecode::pull(2),		//     i++;
		Bytecode::immediate(1),
		Bytecode::add(),
		Bytecode::shove(2),

		Bytecode::pull(2),		// } while(i < 32);
		Bytecode::immediate(1),
		Bytecode::jslt(0),

		Bytecode::drop(1),
	}
};

inline const MockFunction abs
{
	Bytecode::FunctionInfo{.nArgs = 1, .nRet = 1},
	{
		Bytecode::pull(0),
		Bytecode::immediate(0),
		Bytecode::jslt(0),
		Bytecode::pull(0),
		Bytecode::jump(1),
		Bytecode::drop(1),
		Bytecode::label(),
		Bytecode::immediate(0),
		Bytecode::pull(0),
		Bytecode::sub(),
		Bytecode::label(),
		Bytecode::ret()
	}
};

inline const MockFunction factorial
{
	Bytecode::FunctionInfo{.nArgs = 1, .nRet = 1},
	{
		Bytecode::pull(0),
		Bytecode::immediate(1),
		Bytecode::jsgt(0),

		Bytecode::immediate(1),
		Bytecode::ret(),

		Bytecode::label(),

		Bytecode::pull(0),
		Bytecode::immediate(1),
		Bytecode::sub(),

		Bytecode::immediate(0),
		Bytecode::call(1, 1),
		Bytecode::mul(),
		Bytecode::ret()
	}
};

}

#endif /* CODETESTUTILS_H_ */
