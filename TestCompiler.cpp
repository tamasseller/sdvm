#include "jit/Compiler.h"

#include "Disassembler.h"

#include "CodeTestUtils.h"

TEST_GROUP(Compiler)
{
	static inline auto compile(const MockFunction& f)
	{
		uint16_t out[512];
		MockFunction::Reader r(f);
		auto end = Compiler::compile(0, {out, sizeof(out) / sizeof(out[0])}, f.info, r);
		return Disassembler::disassemble(out, end);
	}
};

TEST(Compiler, Square)
{
	auto result = compile(MockFunction
	{
		Bytecode::FunctionInfo{
			.nLabels = 0,
			.nArgs = 1,
			.nRet = 1,
			.hasNonTailCall = false
		},
		{
			Bytecode::pull(0),
			Bytecode::mul()
		}
	});

	checkCodeIs(result,
	{
		"mov r0, lr",
		"blx r9",
		".short 0x0000",
		"add pc, lr",

		"mov r2, r1",
		"muls r1, r2",

		"blx r9",
		".short 0x8001",
	});
}

TEST(Compiler, AddThree)
{
	auto result = compile(MockFunction
	{
		Bytecode::FunctionInfo{
			.nLabels = 0,
			.nArgs = 1,
			.nRet = 1,
			.hasNonTailCall = false
		},
		{
			Bytecode::immediate(3),
			Bytecode::add(),
			Bytecode::ret()
		}
	});

	checkCodeIs(result,
	{
		"mov r0, lr",
		"blx r9",
		".short 0x0000",
		"add pc, lr",

		"movs r2, #3",
		"adds r1, r1, r2",

		"blx r9",
		".short 0x8001",
	});
}

TEST(Compiler, Add1024Times400GreaterThanNegative100)
{
	auto result = compile(MockFunction
	{
		Bytecode::FunctionInfo{
			.nLabels = 1,
			.nArgs = 1,
			.nRet = 1,
			.hasNonTailCall = false
		},
		{
			Bytecode::immediate(1024),
			Bytecode::add(),
			Bytecode::immediate(400),
			Bytecode::mul(),
			Bytecode::immediate(-100),
			Bytecode::jsgt(0),
				Bytecode::immediate(0),
				Bytecode::ret(),
			Bytecode::label(-1),
				Bytecode::immediate(1),
		}
	});

	checkCodeIs(result,
	{
		"mov r0, lr",
		"blx r9",
		".short 0x0000",
		"add pc, lr",

		"movs r2, #1",
		"lsls r2, r2, #10",

		"adds r1, r1, r2",

		"movs r2, #145",
		"adds r2, #255",

		"muls r1, r2",

		"movs r2, #99",
		"mvns r2, r2",

		"cmp r1, r2",
		"bgt 0",

//		"movs r1, #0",			XXX

		"b -2",

//		"movs r1, #1",	        XXX

		"blx r9",
		".short 0x8001",
	});
}

TEST(Compiler, SetLowByte)
{
	auto result = compile(MockFunction
	{
		Bytecode::FunctionInfo{
			.nLabels = 0,
			.nArgs = 2,
			.nRet = 1,
			.hasNonTailCall = false
		},
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
	});

	checkCodeIs(result,
	{
		"mov r0, lr",
		"blx r9",
		".short 0x0000",
		"add pc, lr",

		"movs r4, #8",
		"mov r3, r1",
		"lsrs r3, r4",

		"movs r4, #8",
		"lsls r3, r4",

		"movs r5, #255",
		"mov r4, r2",
		"ands r4, r5",

		"orrs r3, r4",

		"blx r9",
		".short 0x8001",
	});
}

TEST(Compiler, SubXorAsh)
{
	auto result = compile(MockFunction
	{
		Bytecode::FunctionInfo{
			.nLabels = 0,
			.nArgs = 4,
			.nRet = 1,
			.hasNonTailCall = false
		},
		{
			Bytecode::sub(),
			Bytecode::aXor(),
			Bytecode::ash(),
		}
	});

	checkCodeIs(result,
	{
		"mov r0, lr",
		"blx r9",
		".short 0x0000",
		"add pc, lr",

		"ldr r4, [sp, #1020]",
		"subs r3, r3, r4",
		"eors r2, r3",
		"asrs r1, r2",

		"blx r9",
		".short 0x8001",
	});
}

TEST(Compiler, ConsumeFnv)
{
	auto result = compile(MockFunction
	{
		Bytecode::FunctionInfo{
			.nLabels = 0,
			.nArgs = 2,
			.nRet = 1,
			.hasNonTailCall = false
		},
		{
			Bytecode::aXor(),
			Bytecode::immediate(16777619),
			Bytecode::mul(),
			Bytecode::ret()
		}
	});

	checkCodeIs(result,
	{
		"mov r0, lr",
		"blx r9",
		".short 0x0000",
		"add pc, lr",

		"eors r1, r2",
		"ldr r2, [pc, #8]",
		"muls r1, r2",

		"blx r9",
		".short 0x8001",
		"nop",
		".long 0x01000193"
	});
}

TEST(Compiler, PopCount)
{
	auto result = compile(MockFunction
	{
		Bytecode::FunctionInfo{
			.nLabels = 2,
			.nArgs = 1,
			.nRet = 1,
			.hasNonTailCall = false
		},
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
	});

	checkCodeIs(result,
	{
		"mov r0, lr",
		"blx r9",
		".short 0x0000",
		"add pc, lr",

		"movs r5, #0",
		"movs r4, #1",
		"lsls r4, r5",

		"mov r5, r1",
		"ands r4, r5",

		"movs r5, #0",
		"cmp r4, r5",
		"beq 6",

		"movs r5, #1",

		"movs r4, #0",

		"adds r4, r4, r5",

		"mov r1, r3",

		"mov r3, r2",
		"movs r4, #1",

		"adds r3, r3, r4",

		"mov r2, r3",

		"mov r3, r2",

		"movs r4, #1",

		"cmp r3, r4",
		"blt -42",

		"blx r9",
		".short 0x8001",
	});
}
#if 0
TEST(Compiler, Abs)
{
	auto result = compile(MockFunction
	{
		Bytecode::FunctionInfo{
			.nLabels = 2,
			.nArgs = 1,
			.nRet = 1,
			.hasNonTailCall = false
		},
		{
			Bytecode::pull(0),
			Bytecode::immediate(0),
			Bytecode::jslt(0),
			Bytecode::pull(0),
			Bytecode::jump(1),
			Bytecode::label(-1), // 0
			Bytecode::immediate(0),
			Bytecode::pull(0),
			Bytecode::sub(),
			Bytecode::label(0),	// 1
			Bytecode::ret()
		}
	});

	checkCodeIs(result,
	{
		"mov r0, lr",
		"blx r9",
		".short 0x0000",
		"add pc, lr",

		"mov r1, r0",
		"movs r2, #0",
		"cmp r1, r2",
		"blt 2",

		"mov r1, r0",

		"b 4",

		"movs r2, #0",
		"mov r3, r0",
		"subs r2, r2, r3",

		"blx r9",
		".short 0x8001",
	});
}

TEST(Compiler, Factorial)
{
	auto result = compile(MockFunction
	{
		Bytecode::FunctionInfo{
			.nLabels = 1,
			.nArgs = 1,
			.nRet = 1,
			.hasNonTailCall = false
		},
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
	});

	checkCodeIs(result,
	{
		"mov r0, lr",
		"blx r9",
		".short 0x0000",
		"add pc, lr",

		"mov r1, r0",
		"movs r2, #1",
		"cmp r1, r2",
		"bgt 2",
		"movs r1, #1",
		"b 16",

		"mov r2, r0",
		"movs r3, #1",
		"subs r2, r2, r3",

		"movs r3, #0",

		"pop {r0}",
		"add r0, r10",
		"ldr r1, [r0, #0]",
		"blx r1",

		"muls r2, r3",

		"blx r9",
		".short 0x8001",
	});
}
#endif
