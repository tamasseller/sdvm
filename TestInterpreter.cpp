#include "TestUtils.h"

TEST_GROUP(Interpreter){};

TEST(Interpreter, AddImm)
{
	doRunTest({}, {3}, Program{{
		{
			{2, 0, 0, 1},
			{
				{
					Visa::Instruction::imm(1),
					Visa::Instruction::imm(2),
					Visa::Instruction::add(),
					Visa::Instruction::ret()
				}
			}
		}
	}});
}

TEST(Interpreter, SubImmFromArg)
{
	doRunTest({5}, {3}, Program{{
		{
			{2, 0, 1, 1},
			{
				{
					Visa::Instruction::loadArgument(0),
					Visa::Instruction::imm(2),
					Visa::Instruction::sub(),
					Visa::Instruction::ret()
				}
			}
		}
	}});
}

TEST(Interpreter, AddArgs)
{
	doRunTest({1, 2}, {3}, Program{{
		{
			{2, 0, 2, 1},
			{
				{
					Visa::Instruction::loadArgument(0),
					Visa::Instruction::loadArgument(1),
					Visa::Instruction::add(),
					Visa::Instruction::ret()
				}
			}
		}
	}});
}

TEST(Interpreter, ImmStoreLoad)
{
	doRunTest({}, {123}, Program{{
		{
			{1, 1, 0, 1},
			{
				{
					Visa::Instruction::imm(123),
					Visa::Instruction::storeLocal(0),
					Visa::Instruction::loadLocal(0),
					Visa::Instruction::ret()
				}
			}
		}
	}});
}

TEST(Interpreter, Square)
{
	doRunTest({2}, {4}, Program{{
		{
			{2, 0, 1, 1},
			{
				{
					Visa::Instruction::loadArgument(0),
					Visa::Instruction::dup(),
					Visa::Instruction::mul(),
					Visa::Instruction::ret()
				}
			}
		}
	}});
}

TEST(Interpreter, DivMod)
{
	doRunTest({23, 7}, {3, 2}, Program{{
		{
			{3, 0, 2, 2},
			{
				{
					Visa::Instruction::loadArgument(0),
					Visa::Instruction::loadArgument(1),
					Visa::Instruction::div(),
					Visa::Instruction::loadArgument(0),
					Visa::Instruction::loadArgument(1),
					Visa::Instruction::mod(),
					Visa::Instruction::ret()
				}

			}
		}
	}});
}

TEST(Interpreter, Arithmetic)
{
	doRunTest({3, 6}, {9, 3, 18, 2, 3, 2, 7, 5, 48, 0}, Program{{
		{
			{11, 0, 2, 10},
			{
				{
					Visa::Instruction::loadArgument(0),
					Visa::Instruction::loadArgument(1),
					Visa::Instruction::add(),

					Visa::Instruction::loadArgument(1),
					Visa::Instruction::loadArgument(0),
					Visa::Instruction::sub(),

					Visa::Instruction::loadArgument(0),
					Visa::Instruction::loadArgument(1),
					Visa::Instruction::mul(),

					Visa::Instruction::loadArgument(1),
					Visa::Instruction::loadArgument(0),
					Visa::Instruction::div(),

					Visa::Instruction::loadArgument(0),
					Visa::Instruction::loadArgument(1),
					Visa::Instruction::mod(),

					Visa::Instruction::loadArgument(0),
					Visa::Instruction::loadArgument(1),
					Visa::Instruction::aAnd(),

					Visa::Instruction::loadArgument(0),
					Visa::Instruction::loadArgument(1),
					Visa::Instruction::aOr(),

					Visa::Instruction::loadArgument(0),
					Visa::Instruction::loadArgument(1),
					Visa::Instruction::aXor(),

					Visa::Instruction::loadArgument(1),
					Visa::Instruction::loadArgument(0),
					Visa::Instruction::lsh(),

					Visa::Instruction::loadArgument(1),
					Visa::Instruction::loadArgument(0),
					Visa::Instruction::rsh(),

					Visa::Instruction::ret()

				}
			}
		}
	}});
}

TEST(Interpreter, LogicTrue)
{
	doRunTest({}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, Program{{
		{
			{13, 0, 0, 12},
			{
				{
					Visa::Instruction::imm(1),
					Visa::Instruction::imm(1),
					Visa::Instruction::leq(),

					Visa::Instruction::imm(0),
					Visa::Instruction::imm(1),
					Visa::Instruction::llt(),

					Visa::Instruction::imm(1),
					Visa::Instruction::imm(0),
					Visa::Instruction::lgt(),

					Visa::Instruction::imm(1),
					Visa::Instruction::imm(0),
					Visa::Instruction::lne(),

					Visa::Instruction::imm(1),
					Visa::Instruction::imm(0),
					Visa::Instruction::lge(),

					Visa::Instruction::imm(1),
					Visa::Instruction::imm(1),
					Visa::Instruction::lge(),

					Visa::Instruction::imm(0),
					Visa::Instruction::imm(1),
					Visa::Instruction::lle(),

					Visa::Instruction::imm(1),
					Visa::Instruction::imm(1),
					Visa::Instruction::lle(),

					Visa::Instruction::imm(1),
					Visa::Instruction::imm(1),
					Visa::Instruction::lAnd(),

					Visa::Instruction::imm(1),
					Visa::Instruction::imm(1),
					Visa::Instruction::lOr(),

					Visa::Instruction::imm(0),
					Visa::Instruction::imm(1),
					Visa::Instruction::lOr(),

					Visa::Instruction::imm(1),
					Visa::Instruction::imm(0),
					Visa::Instruction::lOr(),

					Visa::Instruction::ret()
				}
			}
		}
	}});
}

TEST(Interpreter, LogicFalse)
{
	doRunTest({}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, Program{{
		{
			{13, 0, 0, 12},
			{
				{
					Visa::Instruction::imm(0),
					Visa::Instruction::imm(1),
					Visa::Instruction::leq(),

					Visa::Instruction::imm(1),
					Visa::Instruction::imm(0),
					Visa::Instruction::llt(),

					Visa::Instruction::imm(1),
					Visa::Instruction::imm(1),
					Visa::Instruction::llt(),

					Visa::Instruction::imm(0),
					Visa::Instruction::imm(1),
					Visa::Instruction::lgt(),

					Visa::Instruction::imm(0),
					Visa::Instruction::imm(0),
					Visa::Instruction::lgt(),

					Visa::Instruction::imm(0),
					Visa::Instruction::imm(0),
					Visa::Instruction::lne(),

					Visa::Instruction::imm(0),
					Visa::Instruction::imm(1),
					Visa::Instruction::lge(),

					Visa::Instruction::imm(1),
					Visa::Instruction::imm(0),
					Visa::Instruction::lle(),

					Visa::Instruction::imm(0),
					Visa::Instruction::imm(0),
					Visa::Instruction::lAnd(),

					Visa::Instruction::imm(1),
					Visa::Instruction::imm(0),
					Visa::Instruction::lAnd(),

					Visa::Instruction::imm(0),
					Visa::Instruction::imm(1),
					Visa::Instruction::lAnd(),

					Visa::Instruction::imm(0),
					Visa::Instruction::imm(0),
					Visa::Instruction::lOr(),

					Visa::Instruction::ret()
				}
			}
		}
	}});
}

TEST(Interpreter, DropLastBits)
{
	doRunTest({7, 2}, {4}, Program{{
		{
			{3, 0, 2, 1},
			{
				{
					Visa::Instruction::loadArgument(0),
					Visa::Instruction::loadArgument(1),
					Visa::Instruction::rsh(),
					Visa::Instruction::loadArgument(1),
					Visa::Instruction::lsh(),
					Visa::Instruction::ret()
				}

			}
		}
	}});
}

TEST(Interpreter, ExpNoLocal)
{
	doRunTest({3, 3}, {27}, Program{{
		{
			{3, 0, 2, 1},
			{
				{
					Visa::Instruction::imm(1),
				},
				{
					Visa::Instruction::loadArgument(1),
					Visa::Instruction::imm(0),
					Visa::Instruction::leq(),
					Visa::Instruction::jcond(2),

					Visa::Instruction::loadArgument(1),
					Visa::Instruction::imm(1),
					Visa::Instruction::sub(),
					Visa::Instruction::storeArgument(1),

					Visa::Instruction::loadArgument(0),
					Visa::Instruction::mul(),
					Visa::Instruction::jmp(1)
				},
				{
					Visa::Instruction::ret()
				}
			}
		}
	}});
}

TEST(Interpreter, ExpLocalLoop)
{
	doRunTest({5, 4}, {625}, Program{{
		{
			{3, 2, 2, 1},
			{
				{
					Visa::Instruction::imm(1),
					Visa::Instruction::storeLocal(0),	// ret=1;
					Visa::Instruction::imm(0),
					Visa::Instruction::storeLocal(1),	// i=0;
				},
				{
					Visa::Instruction::loadLocal(1),		// i < arg1
					Visa::Instruction::loadArgument(1),
					Visa::Instruction::lge(),
					Visa::Instruction::jcond(3),
				},
				{
					Visa::Instruction::loadArgument(0),	// ret *= arg0;
					Visa::Instruction::loadLocal(0),
					Visa::Instruction::mul(),
					Visa::Instruction::storeLocal(0),

					Visa::Instruction::loadLocal(1),		// i++;
					Visa::Instruction::imm(1),
					Visa::Instruction::add(),
					Visa::Instruction::storeLocal(1),

					Visa::Instruction::jmp(1)
				},
				{
					Visa::Instruction::loadLocal(0), 		// return ret;
					Visa::Instruction::ret()
				}
			}
		}
	}});
}

TEST(Interpreter, Factorial)
{
	doRunTest({5}, {120}, Program{{
		{
			{3, 0, 1, 1},
			{
				{
					Visa::Instruction::loadArgument(0),
					Visa::Instruction::imm(1),
					Visa::Instruction::lle(),
					Visa::Instruction::jcond(1),

					Visa::Instruction::loadArgument(0),
					Visa::Instruction::dup(),
					Visa::Instruction::imm(1),
					Visa::Instruction::sub(),

					Visa::Instruction::imm(0),
					Visa::Instruction::call(),
					Visa::Instruction::mul(),

					Visa::Instruction::jmp(2)
				},
				{
					Visa::Instruction::imm(1)
				},
				{
					Visa::Instruction::ret()
				}
			}
		}
	}});
}
