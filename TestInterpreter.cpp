#include "1test/Test.h"

#include "vm/Program.h"

#include <sstream>

TEST_GROUP(Interpreter)
{
	static void doRunTest(const std::vector<uint32_t>& args, const std::vector<uint32_t>& exp, const Program &p)
	{
		const auto result = p.interpret(args);

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
			{2, 0, 0},
			{
				{
					Program::Instruction::imm(1),
					Program::Instruction::imm(2),
					Program::Instruction::add(),
				}
			}
		}
	}});
}

TEST(Interpreter, SubImmFromArg)
{
	doRunTest({5}, {3}, {{
		{
			{2, 0, 1},
			{
				{
					Program::Instruction::loadArgument(0),
					Program::Instruction::imm(2),
					Program::Instruction::sub()
				}
			}
		}
	}});
}

TEST(Interpreter, AddArgs)
{
	doRunTest({1, 2}, {3}, {{
		{
			{2, 0, 2},
			{
				{
					Program::Instruction::loadArgument(0),
					Program::Instruction::loadArgument(1),
					Program::Instruction::add()
				}
			}
		}
	}});
}

TEST(Interpreter, ImmStoreLoad)
{
	doRunTest({}, {123}, {{
		{
			{1, 1, 0},
			{
				{
					Program::Instruction::imm(123),
					Program::Instruction::storeLocal(0),
					Program::Instruction::loadLocal(0)
				}
			}
		}
	}});
}

TEST(Interpreter, Square)
{
	doRunTest({2}, {4}, {{
		{
			{2, 0, 1},
			{
				{
					Program::Instruction::loadArgument(0),
					Program::Instruction::dup(),
					Program::Instruction::mul()
				}
			}
		}
	}});
}

TEST(Interpreter, DivMod)
{
	doRunTest({23, 7}, {3, 2}, {{
		{
			{3, 0, 2},
			{
				{
					Program::Instruction::loadArgument(0),
					Program::Instruction::loadArgument(1),
					Program::Instruction::div(),
					Program::Instruction::loadArgument(0),
					Program::Instruction::loadArgument(1),
					Program::Instruction::mod()
				}
			}
		}
	}});
}

TEST(Interpreter, AndOrXor)
{
	doRunTest({3, 6}, {2, 7, 5}, {{
		{
			{4, 0, 2},
			{
				{
					Program::Instruction::loadArgument(0),
					Program::Instruction::loadArgument(1),
					Program::Instruction::aAnd(),
					Program::Instruction::loadArgument(0),
					Program::Instruction::loadArgument(1),
					Program::Instruction::aOr(),
					Program::Instruction::loadArgument(0),
					Program::Instruction::loadArgument(1),
					Program::Instruction::aXor()
				}
			}
		}
	}});
}

TEST(Interpreter, DropLastBits)
{
	doRunTest({7, 2}, {4}, {{
		{
			{3, 0, 2},
			{
				{
					Program::Instruction::loadArgument(0),
					Program::Instruction::loadArgument(1),
					Program::Instruction::rsh(),
					Program::Instruction::loadArgument(1),
					Program::Instruction::lsh(),
				}
			}
		}
	}});
}

TEST(Interpreter, ExpNoLocal)
{
	doRunTest({3, 3}, {27}, {{
		{
			{3, 0, 2},
			{
				{
					Program::Instruction::imm(1),
				},
				{
					Program::Instruction::loadArgument(1),
					Program::Instruction::imm(0),
					Program::Instruction::jeq(2),

					Program::Instruction::loadArgument(1),
					Program::Instruction::imm(1),
					Program::Instruction::sub(),
					Program::Instruction::storeArgument(1),

					Program::Instruction::loadArgument(0),
					Program::Instruction::mul(),
					Program::Instruction::jmp(1)
				},
				{}
			}
		}
	}});
}

TEST(Interpreter, ExpLocalLoop)
{
	doRunTest({5, 4}, {625}, {{
		{
			{3, 2, 2},
			{
				{
					Program::Instruction::imm(1),
					Program::Instruction::storeLocal(0),	// ret=1;
					Program::Instruction::imm(0),
					Program::Instruction::storeLocal(1),	// i=0;
				},
				{
					Program::Instruction::loadLocal(1),		// i < arg1
					Program::Instruction::loadArgument(1),
					Program::Instruction::jge(3),
				},
				{
					Program::Instruction::loadArgument(0),	// ret *= arg0;
					Program::Instruction::loadLocal(0),
					Program::Instruction::mul(),
					Program::Instruction::storeLocal(0),

					Program::Instruction::loadLocal(1),		// i++;
					Program::Instruction::imm(1),
					Program::Instruction::add(),
					Program::Instruction::storeLocal(1),

					Program::Instruction::jmp(1)
				},
				{
					Program::Instruction::loadLocal(0) 		// return ret;
				}
			}
		}
	}});
}

TEST(Interpreter, Factorial)
{
	doRunTest({5}, {120}, {{
		{
			{3, 0, 1},
			{
				{
					Program::Instruction::loadArgument(0),
					Program::Instruction::imm(1),
					Program::Instruction::jle(1),

					Program::Instruction::loadArgument(0),
					Program::Instruction::dup(),
					Program::Instruction::imm(1),
					Program::Instruction::sub(),

					Program::Instruction::imm(0),
					Program::Instruction::call(),
					Program::Instruction::mul(),

					Program::Instruction::jmp(2),
				},
				{
					Program::Instruction::imm(1),
				},
				{}
			}
		}
	}});
}
