#include "TestUtils.h"

#include "jit/Compiler.h"

#include "Disassembler.h"

class MockFunction
{
	std::vector<Bytecode::Instruction> body;

public:
	const Bytecode::FunctionInfo info;
	inline MockFunction(const Bytecode::FunctionInfo &info, const std::vector<Bytecode::Instruction> &body): body(body), info(info)	{}

	class Reader: public Bytecode::InstructionStreamReader
	{
		decltype(body)::const_iterator it, end;

		static bool read(Bytecode::InstructionStreamReader *isr, Bytecode::Instruction& isn)
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

		"ldr r0, [sp, #12]",
		"push {r0}",

		"pop {r0, r1}",
		"muls r1, r0",
		"push {r1}",

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

		"movs r0, #3",
		"push {r0}",

		"pop {r0, r1}",
		"adds r1, r1, r0",
		"push {r1}",

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
			Bytecode::label(),
				Bytecode::immediate(1),
		}
	});

	checkCodeIs(result,
	{
		"mov r0, lr",
		"blx r9",
		".short 0x0000",
		"add pc, lr",

		"movs r0, #1",
		"lsls r0, r0, #10",
		"push {r0}",

		"pop {r0, r1}",
		"adds r1, r1, r0",
		"push {r1}",

		"movs r0, #145",
		"adds r0, #255",
		"push {r0}",

		"pop {r0, r1}",
		"muls r1, r0",
		"push {r1}",

		"movs r0, #99",
		"mvns r0, r0",
		"push {r0}",

		"pop {r0, r1}",
		"cmp r1, r0",
		"bgt 4",

		"movs r0, #0",
		"push {r0}",

		"b 2",

		"movs r0, #1",
		"push {r0}",

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
			.nArgs = 1,
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

		"ldr r0, [sp, #12]",
		"push {r0}",

		"movs r0, #8",
		"push {r0}",

		"pop {r0, r1}",
		"lsrs r1, r0",
		"push {r1}",

		"movs r0, #8",
		"push {r0}",

		"pop {r0, r1}",
		"lsls r1, r0",
		"push {r1}",

		"ldr r0, [sp, #12]",
		"push {r0}",

		"movs r0, #255",
		"push {r0}",

		"pop {r0, r1}",
		"ands r1, r0",
		"push {r1}",

		"pop {r0, r1}",
		"orrs r1, r0",
		"push {r1}",

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

		"pop {r0, r1}",
		"subs r1, r1, r0",
		"push {r1}",

		"pop {r0, r1}",
		"eors r1, r0",
		"push {r1}",

		"pop {r0, r1}",
		"asrs r1, r0",
		"push {r1}",

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

		"pop {r0, r1}",
		"eors r1, r0",
		"push {r1}",

		"ldr r0, [pc, #12]",
		"push {r0}",

		"pop {r0, r1}",
		"muls r1, r0",
		"push {r1}",

		"blx r9",
		".short 0x8001",
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

		"movs r0, #0",
		"push {r0}",

		"movs r0, #0",
		"push {r0}",

		"movs r0, #1",
		"push {r0}",

		"ldr r0, [sp, #16]",
		"push {r0}",

		"pop {r0, r1}",
		"lsls r1, r0",
		"push {r1}",

		"ldr r0, [sp, #24]",
		"push {r0}",

		"pop {r0, r1}",
		"ands r1, r0",
		"push {r1}",

		"pop {r0, r1}",
		"cmp r1, r0",
		"beq 16",

		"ldr r0, [sp, #12]",
		"push {r0}",

		"movs r0, #1",
		"push {r0}",

		"pop {r0, r1}",
		"adds r1, r1, r0",
		"push {r1}",

		"pop {r0}",
		"str r0, [sp, #12]",

		"ldr r0, [sp, #8]",
		"push {r0}",

		"movs r0, #1",
		"push {r0}",

		"pop {r0, r1}",
		"adds r1, r1, r0",
		"push {r1}",

		"pop {r0}",
		"str r0, [sp, #8]",

		"ldr r0, [sp, #8]",
		"push {r0}",

		"movs r0, #1",
		"push {r0}",

		"pop {r0, r1}",
		"cmp r1, r0",
		"blt -82",

		"add sp, #4",

		"blx r9",
		".short 0x8001",
	});
}

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

		"ldr r0, [sp, #12]",
		"push {r0}",

		"movs r0, #0",
		"push {r0}",

		"pop {r0, r1}",
		"cmp r1, r0",
		"blt 4",

		"ldr r0, [sp, #12]",
		"push {r0}",

		"b 12",

		"movs r0, #0",
		"push {r0}",

		"ldr r0, [sp, #16]",
		"push {r0}",

		"pop {r0, r1}",
		"subs r1, r1, r0",
		"push {r1}",

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

		"ldr r0, [sp, #12]",
		"push {r0}",

		"movs r0, #1",
		"push {r0}",

		"pop {r0, r1}",
		"cmp r1, r0",
		"bgt 4",

		"movs r0, #1",
		"push {r0}",

		"b 30",

		"ldr r0, [sp, #12]",
		"push {r0}",

		"movs r0, #1",
		"push {r0}",

		"pop {r0, r1}",
		"subs r1, r1, r0",
		"push {r1}",

		"movs r0, #0",
		"push {r0}",

		"pop {r0}",
		"add r0, r10",
		"ldr r1, [r0, #0]",
		"blx r1",

		"pop {r0, r1}",
		"muls r1, r0",
		"push {r1}",

		"blx r9",
		".short 0x8001",
	});
}
