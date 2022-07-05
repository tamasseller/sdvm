#include "jit/RegisterAllocator.h"
#include "jit/Assembler.h"

#include "Disassembler.h"

#include "CodeTestUtils.h"

TEST_GROUP(RegisterAllocator) {};

TEST(RegisterAllocator, Sanity)
{
	uint16_t out[512];
	Assembler::LabelInfo labels[100];
	Assembler a(out, sizeof(out)/sizeof(out[0]), labels, sizeof(labels)/sizeof(labels[0]));
	RegisterAllocator ra(0);

	a.emit(ArmV6::movs(ra.acquire(a), 123));
	a.emit(ArmV6::blx(ra.consume(a)));

	auto end = a.assemble();
	auto result = Disassembler::disassemble(out, end);

	checkCodeIs(result,
	{
		"movs r1, #123",
		"blx r1"
	});
}

TEST(RegisterAllocator, Immediates)
{
	uint16_t out[512];
	Assembler::LabelInfo labels[100];
	Assembler a(out, sizeof(out)/sizeof(out[0]), labels, sizeof(labels)/sizeof(labels[0]));
	RegisterAllocator ra(0);

	ra.pushImmediate(a, 123);
	ra.pushImmediate(a, 345);
	ra.pushImmediate(a, 0x234000u);
	ra.pushImmediate(a, -123u);
	ra.pushImmediate(a, 0x12345678u);
	a.emit(ArmV6::blx(ra.consume(a)));
	a.emit(ArmV6::blx(ra.consume(a)));
	a.emit(ArmV6::blx(ra.consume(a)));
	a.emit(ArmV6::blx(ra.consume(a)));
	a.emit(ArmV6::blx(ra.consume(a)));

	auto end = a.assemble();
	auto result = Disassembler::disassemble(out, end);

	checkCodeIs(result,
	{
		"ldr r5, [pc, #24]",
		"blx r5",

		"movs r4, #122",
		"mvns r4, r4",
		"blx r4",

		"movs r3, #141",
		"lsls r3, r3, #14",
		"blx r3",

		"movs r2, #90",
		"adds r2, #255",
		"blx r2",

		"movs r1, #123",
		"blx r1",

		"nop",
		".long 0x12345678"
	});
}

TEST(RegisterAllocator, Arguments)
{
	uint16_t out[512];
	Assembler::LabelInfo labels[100];
	Assembler a(out, sizeof(out)/sizeof(out[0]), labels, sizeof(labels)/sizeof(labels[0]));
	RegisterAllocator ra(6);

	for(int i = 0; i < 6; i++)
	{
		a.emit(ArmV6::blx(ra.consume(a)));
	}

	auto end = a.assemble();
	auto result = Disassembler::disassemble(out, end);

	checkCodeIs(result,
	{
		"ldr r6, [sp, #1012]",
		"blx r6",
		"ldr r5, [sp, #1016]",
		"blx r5",
		"ldr r4, [sp, #1020]",
		"blx r4",
		"blx r3",
		"blx r2",
		"blx r1",
	});
}

TEST(RegisterAllocator, FillNormal)
{
	uint16_t out[512];
	Assembler::LabelInfo labels[100];
	Assembler a(out, sizeof(out)/sizeof(out[0]), labels, sizeof(labels)/sizeof(labels[0]));
	RegisterAllocator ra(8);

	for(int i = 0; i < 5; i++)
	{
		a.emit(ArmV6::movs(ra.acquire(a), 100 + i));
	}

	for(int i = 0; i < 13; i++)
	{
		a.emit(ArmV6::blx(ra.consume(a)));
	}

	auto end = a.assemble();
	auto result = Disassembler::disassemble(out, end);

	checkCodeIs(result,
	{
		"str r1, [sp, #8]",
		"movs r1, #100",

		"str r2, [sp, #4]",
		"movs r2, #101",

		"str r3, [sp, #0]",
		"movs r3, #102",

		"movs r4, #103",

		"movs r5, #104",

		"blx r5",

		"blx r4",

		"blx r3",

		"blx r2",

		"blx r1",

		"ldr r0, [sp, #1004]",
		"blx r0",

		"ldr r7, [sp, #1008]",
		"blx r7",

		"ldr r6, [sp, #1012]",
		"blx r6",

		"ldr r5, [sp, #1016]",
		"blx r5",

		"ldr r4, [sp, #1020]",
		"blx r4",

		"ldr r3, [sp, #0]",
		"blx r3",

		"ldr r2, [sp, #4]",
		"blx r2",

		"ldr r1, [sp, #8]",
		"blx r1"
	});
}

TEST(RegisterAllocator, FillImmediate)
{
	uint16_t out[512];
	Assembler::LabelInfo labels[100];
	Assembler a(out, sizeof(out)/sizeof(out[0]), labels, sizeof(labels)/sizeof(labels[0]));
	RegisterAllocator ra(0);

	for(int i = 0; i < 9; i++)
	{
		ra.pushImmediate(a, 100 + i);
	}

	for(int i = 0; i < 9; i++)
	{
		a.emit(ArmV6::blx(ra.consume(a)));
	}

	auto end = a.assemble();
	auto result = Disassembler::disassemble(out, end);

	checkCodeIs(result,
	{
		"movs r1, #100",
		"str r1, [sp, #1020]",

		"movs r1, #108",
		"blx r1",

		"movs r0, #107",
		"blx r0",

		"movs r7, #106",
		"blx r7",

		"movs r6, #105",
		"blx r6",

		"movs r5, #104",
		"blx r5",

		"movs r4, #103",
		"blx r4",

		"movs r3, #102",
		"blx r3",

		"movs r2, #101",
		"blx r2",

		"ldr r1, [sp, #1020]",
		"blx r1",
	});
}

TEST(RegisterAllocator, PullConsume)
{
	uint16_t out[512];
	Assembler::LabelInfo labels[100];
	Assembler a(out, sizeof(out)/sizeof(out[0]), labels, sizeof(labels)/sizeof(labels[0]));
	RegisterAllocator ra(0);

	a.emit(ArmV6::movs(ra.acquire(a), 123));
	ra.pushImmediate(a, 234);
	ra.pull(a, 0);
	ra.pull(a, 1);
	ra.pull(a, 2);
	a.emit(ArmV6::blx(ra.consume(a)));
	a.emit(ArmV6::blx(ra.consume(a)));
	a.emit(ArmV6::blx(ra.consume(a)));
	a.emit(ArmV6::blx(ra.consume(a)));
	a.emit(ArmV6::blx(ra.consume(a)));

	auto end = a.assemble();
	auto result = Disassembler::disassemble(out, end);

	checkCodeIs(result,
	{
		"movs r1, #123",

		"blx r1",

		"movs r4, #234",
		"blx r4",

		"blx r1",

		"movs r2, #234",
		"blx r2",

		"blx r1",
	});
}

TEST(RegisterAllocator, PullConsumeDeep)
{
	uint16_t out[512];
	Assembler::LabelInfo labels[100];
	Assembler a(out, sizeof(out)/sizeof(out[0]), labels, sizeof(labels)/sizeof(labels[0]));
	RegisterAllocator ra(0);

	for(int i = 0; i < 8; i++)
	{
		ra.pushImmediate(a, 100 + i);
	}

	ra.pull(a, 0);

	for(int i = 0; i < 9; i++)
	{
		a.emit(ArmV6::blx(ra.consume(a)));
	}

	auto end = a.assemble();
	auto result = Disassembler::disassemble(out, end);

	checkCodeIs(result,
	{
		"movs r1, #100",
		"str r1, [sp, #1020]",

		"ldr r1, [sp, #1020]",
		"blx r1",

		"movs r0, #107",
		"blx r0",

		"movs r7, #106",
		"blx r7",

		"movs r6, #105",
		"blx r6",

		"movs r5, #104",
		"blx r5",

		"movs r4, #103",
		"blx r4",

		"movs r3, #102",
		"blx r3",

		"movs r2, #101",
		"blx r2",

		"ldr r1, [sp, #1020]",
		"blx r1"
	});
}

TEST(RegisterAllocator, PullSpillSource)
{
	uint16_t out[512];
	Assembler::LabelInfo labels[100];
	Assembler a(out, sizeof(out)/sizeof(out[0]), labels, sizeof(labels)/sizeof(labels[0]));
	RegisterAllocator ra(1);

	ra.pull(a, 0);

	for(int i = 0; i < 8; i++)
	{
		ra.pushImmediate(a, 100 + i);
	}

	for(int i = 0; i < 10; i++)
	{
		a.emit(ArmV6::blx(ra.consume(a)));
	}

	auto end = a.assemble();
	auto result = Disassembler::disassemble(out, end);

	checkCodeIs(result,
	{
		"mov r2, r1",			// eliminate copies
		"str r1, [sp, #0]",		// spill
		"str r2, [sp, #1020]",	// spill

		"movs r2, #107",
		"blx r2",

		"movs r1, #106",
		"blx r1",

		"movs r0, #105",
		"blx r0",

		"movs r7, #104",
		"blx r7",

		"movs r6, #103",
		"blx r6",

		"movs r5, #102",
		"blx r5",

		"movs r4, #101",
		"blx r4",

		"movs r3, #100",
		"blx r3",

		"ldr r2, [sp, #1020]",
		"blx r2",

		"ldr r1, [sp, #0]",
		"blx r1"
	});
}

TEST(RegisterAllocator, ShoveNormal)
{
	uint16_t out[512];
	Assembler::LabelInfo labels[100];
	Assembler a(out, sizeof(out)/sizeof(out[0]), labels, sizeof(labels)/sizeof(labels[0]));
	RegisterAllocator ra(1);

	ra.pull(a, 0);
	a.emit(ArmV6::movs(ra.acquire(a), 234));
	ra.shove(a, 0);

	a.emit(ArmV6::blx(ra.consume(a)));
	a.emit(ArmV6::blx(ra.consume(a)));

	auto end = a.assemble();
	auto result = Disassembler::disassemble(out, end);

	checkCodeIs(result,
	{
		"movs r3, #234",
		"mov r2, r1",
		"mov r1, r3",

		"blx r2",
		"blx r1",
	});
}

TEST(RegisterAllocator, ShoveImmediate)
{
	uint16_t out[512];
	Assembler::LabelInfo labels[100];
	Assembler a(out, sizeof(out)/sizeof(out[0]), labels, sizeof(labels)/sizeof(labels[0]));
	RegisterAllocator ra(1);

	ra.pull(a, 0);
	ra.pushImmediate(a, 234);
	ra.shove(a, 0);

	a.emit(ArmV6::blx(ra.consume(a)));
	a.emit(ArmV6::blx(ra.consume(a)));

	auto end = a.assemble();
	auto result = Disassembler::disassemble(out, end);

	checkCodeIs(result,
	{
		"mov r2, r1",

		"blx r2",

		"movs r1, #234",
		"blx r1",
	});
}

TEST(RegisterAllocator, ShoveDeep)
{
	uint16_t out[512];
	Assembler::LabelInfo labels[100];
	Assembler a(out, sizeof(out)/sizeof(out[0]), labels, sizeof(labels)/sizeof(labels[0]));
	RegisterAllocator ra(1);

	ra.pull(a, 0);

	for(int i = 0; i < 7; i++)
	{
		ra.pushImmediate(a, 100 + i);
	}

	ra.shove(a, 0);

	for(int i = 0; i < 8; i++)
	{
		a.emit(ArmV6::blx(ra.consume(a)));
	}

	auto end = a.assemble();
	auto result = Disassembler::disassemble(out, end);

	checkCodeIs(result,
	{
		"mov r2, r1",		 // eliminate copies

		"str r1, [sp, #0]",	 // spill

		"movs r1, #106",	 // summonImmediate
		"str r1, [sp, #0]",  // shove deep

		"movs r0, #105",
		"blx r0",

		"movs r7, #104",
		"blx r7",

		"movs r6, #103",
		"blx r6",

		"movs r5, #102",
		"blx r5",

		"movs r4, #101",
		"blx r4",

		"movs r3, #100",
		"blx r3",

		"blx r2",

		"ldr r1, [sp, #0]",
		"blx r1",
	});
}

TEST(RegisterAllocator, ShoveCopyOnSelf)
{
	uint16_t out[512];
	Assembler::LabelInfo labels[100];
	Assembler a(out, sizeof(out)/sizeof(out[0]), labels, sizeof(labels)/sizeof(labels[0]));
	RegisterAllocator ra(0);

	a.emit(ArmV6::movs(ra.acquire(a), 123));
	ra.pull(a, 0);
	ra.shove(a, 0);
	a.emit(ArmV6::blx(ra.consume(a)));

	auto end = a.assemble();
	auto result = Disassembler::disassemble(out, end);

	checkCodeIs(result,
	{
		"movs r1, #123",
		"blx r1",
	});
}

TEST(RegisterAllocator, ShoveCopyBelowSelf)
{
	uint16_t out[512];
	Assembler::LabelInfo labels[100];
	Assembler a(out, sizeof(out)/sizeof(out[0]), labels, sizeof(labels)/sizeof(labels[0]));
	RegisterAllocator ra(0);

	ra.pushImmediate(a, 321);
	a.emit(ArmV6::movs(ra.acquire(a), 123));
	ra.pull(a, 1);
	ra.shove(a, 0);

	ra.pushImmediate(a, 234);

	a.emit(ArmV6::blx(ra.consume(a)));
	a.emit(ArmV6::blx(ra.consume(a)));
	a.emit(ArmV6::blx(ra.consume(a)));

	auto end = a.assemble();
	auto result = Disassembler::disassemble(out, end);

	checkCodeIs(result,
	{
		"movs r2, #123",

		"movs r3, #234",
		"blx r3",

		"mov r1, r2",	// eliminate copies
		"blx r2",

		"blx r1",
	});
}

TEST(RegisterAllocator, ShoveCopyBelowThenDrop)
{
	uint16_t out[512];
	Assembler::LabelInfo labels[100];
	Assembler a(out, sizeof(out)/sizeof(out[0]), labels, sizeof(labels)/sizeof(labels[0]));
	RegisterAllocator ra(0);

	ra.pushImmediate(a, 321);

	a.emit(ArmV6::movs(ra.acquire(a), 123));
	ra.pull(a, 1);
	ra.shove(a, 0);
	ra.drop(a, 1);

	a.emit(ArmV6::blx(ra.consume(a)));

	auto end = a.assemble();
	auto result = Disassembler::disassemble(out, end);

	checkCodeIs(result,
	{
		"movs r2, #123",

		"mov r1, r2",	// eliminate copies

		"blx r1",
	});
}

TEST(RegisterAllocator, UnloadedCopyReplace)
{
	uint16_t out[512];
	Assembler::LabelInfo labels[100];
	Assembler a(out, sizeof(out)/sizeof(out[0]), labels, sizeof(labels)/sizeof(labels[0]));
	RegisterAllocator ra(4);

	ra.pull(a, 3);

	a.emit(ArmV6::blx(ra.replace(a)));

	ra.drop(a, 4);

	auto end = a.assemble();
	auto result = Disassembler::disassemble(out, end);

	checkCodeIs(result,
	{
		"ldr r4, [sp, #1020]",

		"mov r5, r4",

		"blx r5",
	});
}

TEST(RegisterAllocator, UnloadedCopyConsume)
{
	uint16_t out[512];
	Assembler::LabelInfo labels[100];
	Assembler a(out, sizeof(out)/sizeof(out[0]), labels, sizeof(labels)/sizeof(labels[0]));
	RegisterAllocator ra(4);

	ra.pull(a, 3);

	a.emit(ArmV6::blx(ra.consume(a)));

	ra.drop(a, 4);

	auto end = a.assemble();
	auto result = Disassembler::disassemble(out, end);

	checkCodeIs(result,
	{
		"ldr r4, [sp, #1020]",

		"blx r4",
	});
}

TEST(RegisterAllocator, CopyLinking)
{
	uint16_t out[512];
	Assembler::LabelInfo labels[100];
	Assembler a(out, sizeof(out)/sizeof(out[0]), labels, sizeof(labels)/sizeof(labels[0]));
	RegisterAllocator ra(3);

	ra.pull(a, 2);
	ra.shove(a, 0);

	ra.pull(a, 2);
	ra.shove(a, 1);

	a.emit(ArmV6::blx(ra.consume(a)));
	a.emit(ArmV6::blx(ra.consume(a)));
	a.emit(ArmV6::blx(ra.consume(a)));

	auto end = a.assemble();
	auto result = Disassembler::disassemble(out, end);

	checkCodeIs(result,
	{
		"mov r1, r3",
		"blx r3",
		"blx r1",
		"blx r1",
	});
}

TEST(RegisterAllocator, SpillCopy)
{
	uint16_t out[512];
	Assembler::LabelInfo labels[100];
	Assembler a(out, sizeof(out)/sizeof(out[0]), labels, sizeof(labels)/sizeof(labels[0]));
	RegisterAllocator ra(1);

	a.emit(ArmV6::movs(ra.acquire(a), 123));
	ra.pull(a, 1);
	ra.shove(a, 0);

	for(int i = 0; i < 7; i++)
	{
		ra.pushImmediate(a, 100 + i);
	}

	for(int i = 0; i < 9; i++)
	{
		a.emit(ArmV6::blx(ra.consume(a)));
	}

	auto end = a.assemble();
	auto result = Disassembler::disassemble(out, end);

	checkCodeIs(result,
	{
		"movs r2, #123",

		"mov r1, r2",
		"str r1, [sp, #0]",	// spill

		"movs r1, #106",
		"blx r1",

		"movs r0, #105",
		"blx r0",

		"movs r7, #104",
		"blx r7",

		"movs r6, #103",
		"blx r6",

		"movs r5, #102",
		"blx r5",

		"movs r4, #101",
		"blx r4",

		"movs r3, #100",
		"blx r3",

		"blx r2",

		"ldr r1, [sp, #0]",
		"blx r1"
	});
}

TEST(RegisterAllocator, Flush)
{
	uint16_t out[512];
	Assembler::LabelInfo labels[100];
	Assembler a(out, sizeof(out)/sizeof(out[0]), labels, sizeof(labels)/sizeof(labels[0]));
	RegisterAllocator ra(1);

	a.emit(ArmV6::movs(ra.acquire(a), 123));
	ra.pull(a, 1);
	ra.shove(a, 0);
	ra.pushImmediate(a, 234);
	ra.flushDeferred(a);

	auto end = a.assemble();
	auto result = Disassembler::disassemble(out, end);

	checkCodeIs(result,
	{
		"movs r2, #123", // emit
		"mov r1, r2",
		"movs r3, #234", // flush
	});
}

TEST(RegisterAllocator, Ternary)
{
	uint16_t out[512];
	Assembler::LabelInfo labels[100];
	Assembler a(out, sizeof(out)/sizeof(out[0]), labels, sizeof(labels)/sizeof(labels[0]));
	RegisterAllocator ra(2);

	const auto a1 = ra.consume(a);
	const auto a2 = ra.consume(a);

	a.emit(ArmV6::cmp(a2, a1));

	ra.flushDeferred(a);					// jeq +1
	auto s0 = ra.getState();
	a.emit(ArmV6::beq(Assembler::Label(0)));

	ra.pushImmediate(a, 123);				// imm 123

	ra.flushDeferred(a);					// jmp +2
	auto s1 = ra.getState();
	a.emit(ArmV6::b(Assembler::Label(1)));

	ra.drop(a, 1);

	ra.applyState(a, s0);				   // label
	a.pin(0);

	ra.pushImmediate(a, 234);			   // imm 234
	ra.flushDeferred(a);

	ra.applyState(a, s1);			       // label
	a.pin(1);

	auto end = a.assemble();
	auto result = Disassembler::disassemble(out, end);

	checkCodeIs(result,
	{
		"cmp r1, r2",
		"beq 2",
		"movs r1, #123",
		"b 0",
		"movs r1, #234",
	});
}
