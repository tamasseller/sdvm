#include "TestUtils.h"

#include "1test/Mock.h"

struct MockErrorHandler
{
	static inline void take(InterpreterError e) {
		MOCK(Error)::CALL(Error).withParam((int)e);
	}

	static inline void take(InterpreterError e, uint32_t a1, uint32_t a2) {
		MOCK(Error)::CALL(Error).withParam((int)e).withParam(a1).withParam(a2);
	}
};

TEST_GROUP(ErrorHandler)
{
	static inline void execute(const std::vector<uint32_t> &args, const Program& p)
	{
		auto r(p);
		Interpreter<MockErrorHandler>::interpret(r, args);
	}
};

TEST(ErrorHandler, Underflow)
{
	MOCK(Error)::EXPECT(Error).withParam((int)InterpreterError::Underflow);

	execute({}, Program{{
		{
			{0, 0, 0, 0},
			{
				{
					Visa::Instruction::jcond(0),
					Visa::Instruction::ret()
				}
			}
		}
	}});
}

TEST(ErrorHandler, Overflow)
{
	MOCK(Error)::EXPECT(Error).withParam((int)InterpreterError::Overflow);

	execute({}, Program{{
		{
			{0, 0, 0, 0},
			{
				{
					Visa::Instruction::imm(0),
					Visa::Instruction::ret()
				}
			}
		}
	}});
}

TEST(ErrorHandler, LocaLoadOob)
{
	MOCK(Error)::EXPECT(Error).withParam((int)InterpreterError::LocaLoadOob).withParam(0).withParam(0);

	execute({}, Program{{
		{
			{1, 0, 0, 1},
			{
				{
					Visa::Instruction::loadLocal(0),
					Visa::Instruction::ret()
				}
			}
		}
	}});
}

TEST(ErrorHandler, LocaStoreOob)
{
	MOCK(Error)::EXPECT(Error).withParam((int)InterpreterError::LocaStoreOob).withParam(0).withParam(0);

	execute({}, Program{{
		{
			{1, 0, 0, 0},
			{
				{
					Visa::Instruction::imm(0),
					Visa::Instruction::storeLocal(0),
					Visa::Instruction::ret()
				}
			}
		}
	}});
}

TEST(ErrorHandler, ArgumentLoadOob)
{
	MOCK(Error)::EXPECT(Error).withParam((int)InterpreterError::ArgumentLoadOob).withParam(0).withParam(0);

	execute({}, Program{{
		{
			{1, 0, 0, 1},
			{
				{
					Visa::Instruction::loadArgument(0),
					Visa::Instruction::ret()
				}
			}
		}
	}});
}

TEST(ErrorHandler, ArgumentStoreOob)
{
	MOCK(Error)::EXPECT(Error).withParam((int)InterpreterError::ArgumentStoreOob).withParam(0).withParam(0);

	execute({}, Program{{
		{
			{1, 0, 0, 0},
			{
				{
					Visa::Instruction::imm(0),
					Visa::Instruction::storeArgument(0),
					Visa::Instruction::ret()
				}
			}
		}
	}});
}

TEST(ErrorHandler, Unknown1)
{
	MOCK(Error)::EXPECT(Error).withParam((int)InterpreterError::UnknownOperation);

	execute({}, Program{{
		{
			{0, 0, 0, 0},
			{
				{
					{(Visa::OperationGroup)-1},
					Visa::Instruction::ret()
				}
			}
		}
	}});
}

TEST(ErrorHandler, Unknown2)
{
	MOCK(Error)::EXPECT(Error).withParam((int)InterpreterError::UnknownOperation);

	Visa::Instruction i = {.group = Visa::OperationGroup::Move};
	i.dest = (Visa::MoveDetails) -1;

	execute({}, Program{{
		{
			{1, 0, 0, 0},
			{
				{
					Visa::Instruction::imm(10),
					i,
					Visa::Instruction::ret()
				}
			}
		}
	}});
}

TEST(ErrorHandler, Unknown3)
{
	MOCK(Error)::EXPECT(Error).withParam((int)InterpreterError::UnknownOperation);

	Visa::Instruction i = {.group = Visa::OperationGroup::Binary};
	i.binOp = (Visa::BinaryOperation) -1;

	execute({}, Program{{
		{
			{2, 0, 0, 0},
			{
				{
					Visa::Instruction::imm(0),
					Visa::Instruction::dup(),
					i,
					Visa::Instruction::ret()
				}
			}
		}
	}});
}

TEST(ErrorHandler, NotEnoughArguments)
{
	MOCK(Error)::EXPECT(Error).withParam((int)InterpreterError::NotEnoughArguments).withParam(0).withParam(1);

	execute({}, Program{{
		{
			{1, 0, 0, 0},
			{
				{
					Visa::Instruction::imm(1),
					Visa::Instruction::call(),
					Visa::Instruction::ret()
				}
			}
		},
		{
			{1, 0, 1, 0}, {{
				Visa::Instruction::ret()
			}}
		}
	}});
}

TEST(ErrorHandler, WrongReturnValues)
{
	MOCK(Error)::EXPECT(Error).withParam((int)InterpreterError::WrongReturnValues).withParam(0).withParam(1);

	execute({}, Program{{
		{
			{1, 0, 0, 1},
			{
				{
					Visa::Instruction::ret()
				}
			}
		},
	}});
}

TEST(ErrorHandler, NoReturnAtEnd)
{
	MOCK(Error)::EXPECT(Error).withParam((int)InterpreterError::NoReturnAtEnd);

	execute({}, Program{{
		{
			{0, 0, 0, 0},
			{
				{
				}
			}
		},
	}});
}

TEST(ErrorHandler, Exception)
{
	MOCK(Error)::EXPECT(Catch).withParam(1);
	MOCK(Error)::EXPECT(Catch).withParam(2);
	MOCK(Error)::EXPECT(Catch).withParam(3);

	static constexpr const char* errorStrings[] =
	{
#define X(sym, str) str,
		X_INTERPRETER_ERRORS()
#undef X
	};

	try
	{
		ExceptionalErrorHandler::take(InterpreterError::Overflow);
	}
	catch(const ExceptionalErrorHandler::Exception &e)
	{
		CHECK(std::string(e.what()) == errorStrings[(int)InterpreterError::Overflow]);
		MOCK(Error)::CALL(Catch).withParam(1);
	}

	try
	{
		ExceptionalErrorHandler::take(InterpreterError::ArgumentLoadOob, 1, 2);
	}
	catch(const ExceptionalErrorHandler::Exception &e)
	{
		char buff[1234];
		sprintf(buff, errorStrings[(int)InterpreterError::ArgumentLoadOob], 1, 2);
		CHECK(std::string(e.what()) == buff);
		MOCK(Error)::CALL(Catch).withParam(2);
	}

	try
	{
		ExceptionalErrorHandler::take((InterpreterError)-1);
	}
	catch(const ExceptionalErrorHandler::Exception &e)
	{
		CHECK(std::string(e.what()) == "Unknown error");
		MOCK(Error)::CALL(Catch).withParam(3);
	}

}
