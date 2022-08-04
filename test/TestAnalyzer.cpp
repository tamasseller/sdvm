#include "Analyzer.h"

#include "CodeTestUtils.h"

TEST_GROUP(Analyzer)
{
	static inline auto analyzeValid(const MockFunction& f)
	{
		MockFunction::Reader r(f);

		Analyzer::Result ret;
		CHECK(Analyzer::analyze(ret, &r));

		return ret;
	}

	static inline void analyzeBad(const MockFunction& f)
	{
		MockFunction::Reader r(f);

		Analyzer::Result ret;
		CHECK(!Analyzer::analyze(ret, &r));
	}
};

TEST(Analyzer, BadPull)
{
	analyzeBad({Bytecode::FunctionInfo{.nArgs = 0, .nRet = 0},
		{
			Bytecode::pull(0)
		}
	});
}

TEST(Analyzer, BadShove)
{
	analyzeBad({Bytecode::FunctionInfo{.nArgs = 0, .nRet = 0},
		{
			Bytecode::shove(0)
		}
	});

	analyzeBad({Bytecode::FunctionInfo{.nArgs = 1, .nRet = 0},
		{
			Bytecode::shove(0)
		}
	});
}

TEST(Analyzer, BadCall)
{
	analyzeBad({Bytecode::FunctionInfo{.nArgs = 0, .nRet = 0},
		{
			Bytecode::call(1, 0)
		}
	});
}

TEST(Analyzer, BadRet)
{
	analyzeBad({Bytecode::FunctionInfo{.nArgs = 0, .nRet = 1},
		{
			Bytecode::ret()
		}
	});

	analyzeBad({Bytecode::FunctionInfo{
			.nArgs = 0,
			.nRet = 1,
		},
		{}
	});
}

TEST(Analyzer, Underrun)
{
	analyzeBad({Bytecode::FunctionInfo{.nArgs = 0, .nRet = 0},
		{
			Bytecode::add()
		}
	});
}

TEST(Analyzer, BadJump)
{
	analyzeBad({Bytecode::FunctionInfo{.nArgs = 0, .nRet = 0},
		{
			Bytecode::jump(0)
		}
	});
}

TEST(Analyzer, StackMismatch)
{
	analyzeBad({Bytecode::FunctionInfo{.nArgs = 0, .nRet = 0},
		{
			Bytecode::label(),
			Bytecode::immediate(0xbad),
			Bytecode::jump(0)
		}
	});

	analyzeBad({Bytecode::FunctionInfo{
			.nArgs = 1,
			.nRet = 0,
		},
		{
			Bytecode::immediate(0),
			Bytecode::jne(0),
			Bytecode::immediate(1),
			Bytecode::label(),
		}
	});
}

TEST(Analyzer, Square)
{
	auto result = analyzeValid(TestCode::square);
	CHECK(result.maxStack == 2);
	CHECK(result.nLabels == 0);
}

TEST(Analyzer, AddThree)
{
	auto result = analyzeValid(TestCode::addThree);
	CHECK(result.maxStack == 2);
	CHECK(result.nLabels == 0);
}

TEST(Analyzer, Add123)
{
	auto result = analyzeValid(TestCode::add123);
	CHECK(result.maxStack == 2);
	CHECK(result.nLabels == 0);
}

TEST(Analyzer, Add3reverse)
{
	auto result = analyzeValid(TestCode::add3reverse);
	CHECK(result.maxStack == 3);
	CHECK(result.nLabels == 0);
}

TEST(Analyzer, SubThree)
{
	auto result = analyzeValid(TestCode::subThree);
	CHECK(result.maxStack == 2);
	CHECK(result.nLabels == 0);
}

TEST(Analyzer, Sub123)
{
	auto result = analyzeValid(TestCode::sub123);
	CHECK(result.maxStack == 2);
	CHECK(result.nLabels == 0);
}

TEST(Analyzer, Add1024Times400GreaterThanNegative100)
{
	auto result = analyzeValid(TestCode::add1024Times400GreaterThanNegative100);
	CHECK(result.maxStack == 2);
	CHECK(result.nLabels == 1);
}

TEST(Analyzer, SetLowByte)
{
	auto result = analyzeValid(TestCode::setLowByte);
	CHECK(result.maxStack == 5);
	CHECK(result.nLabels == 0);
}

TEST(Analyzer, SubXorAsh)
{
	auto result = analyzeValid(TestCode::subXorAsh);
	CHECK(result.maxStack == 4);
	CHECK(result.nLabels == 0);
}

TEST(Analyzer, ConsumeFnv)
{
	auto result = analyzeValid(TestCode::consumeFnv);
	CHECK(result.maxStack == 2);
	CHECK(result.nLabels == 0);
}

TEST(Analyzer, PopCount)
{
	auto result = analyzeValid(TestCode::popCount);
	CHECK(result.maxStack == 5);
	CHECK(result.nLabels == 2);
}

TEST(Analyzer, Abs)
{
	auto result = analyzeValid(TestCode::abs);
	CHECK(result.maxStack == 3);
	CHECK(result.nLabels == 2);
}

TEST(Analyzer, Factorial)
{
	auto result = analyzeValid(TestCode::factorial);
	CHECK(result.maxStack == 3);
	CHECK(result.nLabels == 1);
}

