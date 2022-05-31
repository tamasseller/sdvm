#include "1test/Test.h"

#include "Interpreter.h"

#include <sstream>

TEST_GROUP(Interpreter)
{
	static void doRunTest(const std::vector<uint32_t>& args, const std::vector<uint32_t>& exp, const Program &p)
	{
		const auto result = Interpreter(p).run(args);

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
};

TEST(Interpreter, AddImm)
{
	doRunTest({}, {3}, {{
		{
			2, 0, 0,
			{
				{
					{Program::Operation::Immediate, 1},
					{Program::Operation::Immediate, 2},
					{Program::Operation::Add}
				}
			}
		}
	}});
}

TEST(Interpreter, SubImmFromArg)
{
	doRunTest({5}, {3}, {{
		{
			2, 0, 1,
			{
				{
					{Program::Operation::Load, 0},
					{Program::Operation::Immediate, 2},
					{Program::Operation::Sub}
				}
			}
		}
	}});
}

TEST(Interpreter, AddArgs)
{
	doRunTest({1, 2}, {3}, {{
		{
			2, 0, 2,
			{
				{
					{Program::Operation::Load, 0},
					{Program::Operation::Load, 1},
					{Program::Operation::Add}
				}
			}
		}
	}});
}

TEST(Interpreter, ImmStoreLoad)
{
	doRunTest({}, {123}, {{
		{
			1, 1, 0,
			{
				{
					{Program::Operation::Immediate, 123},
					{Program::Operation::Store, 0},
					{Program::Operation::Load, 0}
				}
			}
		}
	}});
}

TEST(Interpreter, Square)
{
	doRunTest({2}, {4}, {{
		{
			2, 0, 1,
			{
				{
					{Program::Operation::Load, 0},
					{Program::Operation::Duplicate},
					{Program::Operation::Mul, 0}
				}
			}
		}
	}});
}

TEST(Interpreter, DivMod)
{
	doRunTest({23, 7}, {3, 2}, {{
		{
			3, 0, 2,
			{
				{
					{Program::Operation::Load, 0},
					{Program::Operation::Load, 1},
					{Program::Operation::Div, 0},
					{Program::Operation::Load, 0},
					{Program::Operation::Load, 1},
					{Program::Operation::Mod, 0}
				}
			}
		}
	}});
}

TEST(Interpreter, AndOrXor)
{
	doRunTest({3, 6}, {2, 7, 5}, {{
		{
			4, 0, 2,
			{
				{
					{Program::Operation::Load, 0},
					{Program::Operation::Load, 1},
					{Program::Operation::And, 0},
					{Program::Operation::Load, 0},
					{Program::Operation::Load, 1},
					{Program::Operation::Or, 0},
					{Program::Operation::Load, 0},
					{Program::Operation::Load, 1},
					{Program::Operation::Xor, 0}
				}
			}
		}
	}});
}

TEST(Interpreter, DropLastBits)
{
	doRunTest({7, 2}, {4}, {{
		{
			3, 0, 2,
			{
				{
					{Program::Operation::Load, 0},
					{Program::Operation::Load, 1},
					{Program::Operation::Rshift, 0},
					{Program::Operation::Load, 1},
					{Program::Operation::Lshift, 0},
				}
			}
		}
	}});
}

TEST(Interpreter, Skip)
{
	doRunTest({3}, {3}, {{
		{
			3, 0, 1,
			{
				{
					{Program::Operation::Load, 0},
					{Program::Operation::Store, 1},
					{Program::Operation::Rshift, 0},
					{Program::Operation::Load, 1},
					{Program::Operation::Lshift, 0},
				}
			}
		}
	}});
}

