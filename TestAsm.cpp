#include "TestUtils.h"

#include "jit/asm.h"

TEST_GROUP(Asm)
{
	ArmV6::LoReg r0, r1, r2, r3, r4, r5, r6, r7;
	ArmV6::AnyReg r8, r9, r10, r11, r12, sp, lr, pc;

	TEST_SETUP()
	{
		r0  = {0};
		r1  = {1};
		r2  = {2};
		r3  = {3};
		r4  = {4};
		r5  = {5};
		r6  = {6};
		r7  = {7};
		r8  = {8};
		r9  = {9};
		r10 = {10};
		r11 = {11};
		r12 = {12};
		sp  = {13};
		lr  = {14};
		pc  = {15};
	}
};

TEST(Asm, Straight)
{
	uint16_t code[6] alignas(4);
	Assembler a(code, sizeof(code)/sizeof(code[0]), nullptr, 0);

	a.emit(ArmV6::movs(r0, 16));
	a.emit(ArmV6::add(r0, sp));
	a.emit(ArmV6::ldmia(r0, ArmV6::LoRegs{}.add(r0).add(r1)));
	a.emit(ArmV6::add(r0, r1));
	a.emit(ArmV6::strSp(r0, 16));
	a.bodyDone();

	CHECK(code[0] == 0x2010); // movs r0, #16
	CHECK(code[1] == 0x4468); // add r0, sp
	CHECK(code[2] == 0xc803); // ldmia r0, {r0, r1}
	CHECK(code[3] == 0x4408); // add r0, r1
	CHECK(code[4] == 0x9004); // str r0, [sp, #16]
}

TEST(Asm, If)
{
	uint16_t code[4] alignas(4), *labels[1];
	Assembler a(code, sizeof(code) / sizeof(code[0]), labels, sizeof(labels) / sizeof(labels[0]));

	Assembler::Label g(0);
	a.emit(ArmV6::cmp(r0, r1));
	a.emit(ArmV6::bne(g));
	a.emit(ArmV6::adds(r0, 1));
	a.label(g);

	a.bodyDone();

	CHECK(code[0] == 0x4288); // cmp r0, r1
	CHECK(code[1] == 0xd100); // bne.n 6 <g>
	CHECK(code[2] == 0x3001); // adds r0, #1
}

TEST(Asm, IfElse)
{
	uint16_t code[6] alignas(4), *labels[2];
	Assembler a(code, sizeof(code) / sizeof(code[0]), labels, sizeof(labels) / sizeof(labels[0]));

	Assembler::Label f(0), g(1);
	a.emit(ArmV6::cmp(r0, r1));
	a.emit(ArmV6::bne(f));
	a.emit(ArmV6::adds(r0, 1));
	a.emit(ArmV6::b(g));
	a.label(f);
	a.emit(ArmV6::subs(r1, 2));
	a.label(g);

	a.bodyDone();

	CHECK(code[0] == 0x4288); // cmp r0, r1
	CHECK(code[1] == 0xd101); // bne.n 8 <f>
	CHECK(code[2] == 0x3001); // adds r0, #1
	CHECK(code[3] == 0xe000); // b.n a <g>
	CHECK(code[4] == 0x3902); // subs r1, #2
}

TEST(Asm, Loop)
{
	uint16_t code[6] alignas(4), *labels[2];
	Assembler a(code, sizeof(code) / sizeof(code[0]), labels, sizeof(labels) / sizeof(labels[0]));

	Assembler::Label f(0), g(1);
	a.label(f);
	a.emit(ArmV6::cmp(r0, r1));
	a.emit(ArmV6::beq(g));
	a.emit(ArmV6::adds(r0, 1));
	a.emit(ArmV6::b(f));
	a.label(g);

	a.bodyDone();

	CHECK(code[0] == 0x4288); // cmp r0, r1
	CHECK(code[1] == 0xd001); // beq.n 8 <g>
	CHECK(code[2] == 0x3001); // adds r0, #1
	CHECK(code[3] == 0xe7fb); // b.n 0 <f>
}

TEST(Asm, LongCond)
{
	uint16_t code[1004] alignas(4), *labels[2];
	Assembler a(code, sizeof(code) / sizeof(code[0]), labels, sizeof(labels) / sizeof(labels[0]));

	Assembler::Label f(0), g(1);
	a.label(f);
	a.emit(ArmV6::bhi(g));

	for(int i = 0; i < 1000; i++)
	{
		a.emit(ArmV6::lsls(r0, r0, 0));
	}

	a.label(g);
	a.emit(ArmV6::bls(f));

	a.bodyDone();

	CHECK(code[0] == 0xd900); // bls.n 4 <t>
	CHECK(code[1] == 0xe3e7); // b.n 7d4 <g>

	for(int i = 2; i < 1002; i++)
	{
		CHECK(code[i] == 0x0000); // lsls r0, 0
	}

	CHECK(code[1002] == 0xd800); // bhi.n 7d8 <u>
	CHECK(code[1003] == 0xe413); // b.n 0 <f>
}

TEST(Asm, MixCondLength)
{
	uint16_t code[310] alignas(4), *labels[4];
	Assembler a(code, sizeof(code) / sizeof(code[0]), labels, sizeof(labels) / sizeof(labels[0]));

	Assembler::Label l(0), m(1), n(2), o(3);
	a.label(l);
	a.emit(ArmV6::bhi(m));
	a.emit(ArmV6::beq(n));
	a.label(m);
	a.emit(ArmV6::b(m));

	for(int i = 0; i < 300; i++)
	{
		a.emit(ArmV6::lsls(r0, r0, 0));
	}

	a.label(n);
	a.emit(ArmV6::bmi(o));
	a.emit(ArmV6::bls(l));
	a.label(o);

	a.bodyDone();

	CHECK(code[0] == 0xd801); // bhi.n 6 <m>
	CHECK(code[1] == 0xd100); // bne.n 6 <m>
	CHECK(code[2] == 0xe12c); // b.n 260 <n>
	CHECK(code[3] == 0xe7fe); // b.n 6 <m>

	for(int i = 4; i < 304; i++)
	{
		CHECK(code[i] == 0x0000); // lsls r0, 0
	}

	CHECK(code[304] == 0xd401); // bmi.n 266 <o>
	CHECK(code[305] == 0xd800); // bhi.n 266 <o>
	CHECK(code[306] == 0xe6cc); // b.n 0 <l>
}

TEST(Asm, LiteralSane)
{
	uint16_t code[4] alignas(4);
	Assembler a(code, sizeof(code) / sizeof(code[0]), nullptr, 0);

	a.emit(ArmV6::ldrPc(r0, Assembler::Literal(0)));
	a.bodyDone();
	a.literal(0xb16b00b5);

	CHECK(code[0] == 0x4800); // ldr r0, [pc, #0] ; (4 <a>)
	CHECK(code[1] == 0xbf00); // nop
	CHECK(code[2] == 0x00b5); // .word	0xb16b00b5
	CHECK(code[3] == 0xb16b);
}

TEST(Asm, ManyLiterals)
{
	uint16_t code[12] alignas(4);
	Assembler a(code, sizeof(code) / sizeof(code[0]), nullptr, 0);

	Assembler::Literal la(0), lb(1);
	a.emit(ArmV6::addPc(r0, la));
	a.emit(ArmV6::ldrPc(r1, lb));
	a.emit(ArmV6::ldrPc(r2, la));
	a.emit(ArmV6::addPc(r3, la));
	a.emit(ArmV6::addPc(r4, lb));
	a.emit(ArmV6::ldrPc(r5, la));
	a.emit(ArmV6::ldrPc(r6, lb));
	a.emit(ArmV6::addPc(r7, lb));
	a.bodyDone();
	a.literal(0xb16b00b5);
	a.literal(0x1337c0de);

	CHECK(code[0] == 0xa003);  // add r0, pc, #12 ; (adr r0, 10 <a>)
	CHECK(code[1] == 0x4904);  // ldr r1, [pc, #16] ; (14 <b>)
	CHECK(code[2] == 0x4a02);  // ldr r2, [pc, #8] ; (10 <a>)
	CHECK(code[3] == 0xa302);  // add r3, pc, #8 ; (adr r3, 10 <a>)
	CHECK(code[4] == 0xa402);  // add r4, pc, #8 ; (adr r4, 14 <b>)
	CHECK(code[5] == 0x4d01);  // ldr r5, [pc, #4] ; (10 <a>)
	CHECK(code[6] == 0x4e01);  // ldr r6, [pc, #4] ; (14 <b>)
	CHECK(code[7] == 0xa701);  // add r7, pc, #4 ; (adr r7, 14 <b>)
	CHECK(code[8] == 0x00b5);  // .word 0xb16b00b5
	CHECK(code[9] == 0xb16b);  //
	CHECK(code[10] == 0xc0de); // .word 0x1337c0de
	CHECK(code[11] == 0x1337); //
}
