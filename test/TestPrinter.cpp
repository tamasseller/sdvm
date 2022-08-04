#include "Printer.h"

#include "1test/Test.h"

#include <iostream> // XXX

TEST_GROUP(Printer) {};

TEST(Printer, Empty)
{
	CHECK(Printer::print(Program{{
		{
			{1,2,3,4},
			{
				{
					Visa::Instruction::ret()
				}
			}
		}
	}}) ==
R"(# fun#0
#    stack:  1
#    locals: 2
#    args:   3
#    ret:    4
l0:
  ret 

)");

}

TEST(Printer, AllArithmetic)
{
	CHECK(Printer::print(Program{{
		{
			{4,3,2,1},
			{
				{
					Visa::Instruction::add(),
					Visa::Instruction::sub(),
					Visa::Instruction::mul(),
					Visa::Instruction::div(),
					Visa::Instruction::mod(),
					Visa::Instruction::aAnd(),
					Visa::Instruction::aOr(),
					Visa::Instruction::aXor(),
					Visa::Instruction::lsh(),
					Visa::Instruction::rsh(),
					Visa::Instruction::leq(),
					Visa::Instruction::lne(),
					Visa::Instruction::lgt(),
					Visa::Instruction::lle(),
					Visa::Instruction::llt(),
					Visa::Instruction::lge(),
					Visa::Instruction::lAnd(),
					Visa::Instruction::lOr()
				}
			}
		}
	}})
			==
R"(# fun#0
#    stack:  4
#    locals: 3
#    args:   2
#    ret:    1
l0:
  add
  sub
  mul
  div
  mod
  and
  ior
  xor
  lsh
  rsh
  eq
  neq
  gt
  le
  lt
  gt
  land
  lor

)");
}

TEST(Printer, AllNonArithmetic)
{
	CHECK(Printer::print(Program{{
		{
			{3,5,7,9},
			{
				{
					Visa::Instruction::imm(0x1234),
					Visa::Instruction::dup(),
					Visa::Instruction::jmp(137),
					Visa::Instruction::jcond(420),
					Visa::Instruction::loadArgument(69),
					Visa::Instruction::storeArgument(69),
					Visa::Instruction::loadLocal(1337),
					Visa::Instruction::storeLocal(1337),
					Visa::Instruction::call(),
					Visa::Instruction::ret()
				}
			}
		}
	}})
	==
R"(# fun#0
#    stack:  3
#    locals: 5
#    args:   7
#    ret:    9
l0:
  imm 0x1234
  dup
  jmp l137
  jcond 420
  larg 69
  sarg 69
  lloc 1337
  sloc 1337
  call 
  ret 

)");
}

TEST(Printer, MultiFunctionMultBlock)
{
	CHECK(Printer::print(Program{{
		{
			{1,0,1,1},
			{
				{
					Visa::Instruction::jmp(1),
				},
				{
					Visa::Instruction::imm(0),
					Visa::Instruction::call(),
				},
				{
					Visa::Instruction::jmp(0),
				}
			},
		},
		{
			{0,1,0,0},
			{
				{
					Visa::Instruction::imm(1),
					Visa::Instruction::call(),
				},
				{
					Visa::Instruction::ret(),
				}
			}

		}
	}})
	==
R"(# fun#0
#    stack:  1
#    locals: 0
#    args:   1
#    ret:    1
l0:
  jmp l1
l1:
  imm 0x0
  call 
l2:
  jmp l0

# fun#1
#    stack:  0
#    locals: 1
#    args:   0
#    ret:    0
l0:
  imm 0x1
  call 
l1:
  ret 

)");
}
