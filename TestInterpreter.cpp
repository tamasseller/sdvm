#include "1test/Test.h"

#include "vm/Interpreter.h"

#include <sstream>

struct ProgramReader
{
	using Block = std::vector<Program::Instruction>;
	using Body = std::vector<Block>;

	struct Function
	{
		Program::FrameInfo info;
		Body body;
	};

	std::vector<Function> functions;
	decltype(functions.cbegin()) fIt;
	decltype(fIt->body.cbegin()) bIt;
	decltype(bIt->cbegin()) iIt;

	void init(Program::FrameInfo &fInfo)
	{
		fIt = functions.cbegin();
		bIt = fIt->body.cbegin();
		iIt = bIt->cbegin();

		fInfo = fIt->info;
	}

	struct RestorePoint
	{
		uint32_t fIdx, bIdx, iIdx;
	};

	RestorePoint openFunction(uint32_t idx, Program::FrameInfo &fInfo)
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

	void restore(RestorePoint& rp, Program::FrameInfo& fInfo)
	{
		fIt = functions.cbegin() + rp.fIdx;
		bIt = fIt->body.cbegin() + rp.bIdx;
		iIt = bIt->cbegin() + rp.iIdx;

		fInfo = fIt->info;
	}

	void seekBlock(uint32_t idx)
	{
		if(fIt->body.size() <= idx)
		{
			throw std::runtime_error("Jump to unknown block: " + std::to_string(idx) + " (should be below " + std::to_string(fIt->body.size()));
		}

		bIt = fIt->body.cbegin() + idx;
		iIt = bIt->cbegin();
	}

	bool readNext(Program::Instruction& isn)
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
};

TEST_GROUP(Interpreter)
{
	static void doRunTest(const std::vector<uint32_t>& args, const std::vector<uint32_t>& exp, const ProgramReader &p)
	{
		std::vector<uint32_t> result;

		try
		{
			auto r(p);
			result = Interpreter::interpret(r, args);
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
					Program::Instruction::ret()
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
					Program::Instruction::sub(),
					Program::Instruction::ret()
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
					Program::Instruction::add(),
					Program::Instruction::ret()
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
					Program::Instruction::loadLocal(0),
					Program::Instruction::ret()
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
					Program::Instruction::mul(),
					Program::Instruction::ret()
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
					Program::Instruction::mod(),
					Program::Instruction::ret()
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
					Program::Instruction::aXor(),
					Program::Instruction::ret()
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
					Program::Instruction::ret()
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
				{
					Program::Instruction::ret()
				}
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
					Program::Instruction::loadLocal(0), 		// return ret;
					Program::Instruction::ret()
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

					Program::Instruction::jmp(2)
				},
				{
					Program::Instruction::imm(1)
				},
				{
					Program::Instruction::ret()
				}
			}
		}
	}});
}
