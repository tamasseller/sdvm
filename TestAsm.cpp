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
	uint16_t code[5];
	Assembler a(code, sizeof(code)/sizeof(code[0]), nullptr, 0);

	a.emit(ArmV6::movs(r0, 16));
	a.emit(ArmV6::add(r0, sp));
	a.emit(ArmV6::ldmia(r0, ArmV6::LoRegs{}.add(r0).add(r1)));
	a.emit(ArmV6::add(r0, r1));
	a.emit(ArmV6::strSp(r0, 16));
	a.commit();

	CHECK(code[0] == 0x2010); // movs r0, #16
	CHECK(code[1] == 0x4468); // add r0, sp
	CHECK(code[2] == 0xc803); // ldmia r0, {r0, r1}
	CHECK(code[3] == 0x4408); // add r0, r1
	CHECK(code[4] == 0x9004); // str r0, [sp, #16]
}

TEST(Asm, If)
{
	uint16_t code[3], *labels[1];
	Assembler a(code, sizeof(code) / sizeof(code[0]), labels, sizeof(labels) / sizeof(labels[0]));

	Assembler::Label g(0);
	a.emit(ArmV6::cmp(r0, r1));
	a.emit(ArmV6::bne(g));
	a.emit(ArmV6::adds(r0, 1));
	a.label(g);

	a.commit();

	CHECK(code[0] == 0x4288); // cmp r0, r1
	CHECK(code[1] == 0xd100); // bne.n 6 <g>
	CHECK(code[2] == 0x3001); // adds r0, #1
}

TEST(Asm, IfElse)
{
	uint16_t code[5], *labels[2];
	Assembler a(code, sizeof(code) / sizeof(code[0]), labels, sizeof(labels) / sizeof(labels[0]));

	Assembler::Label f(0), g(1);
	a.emit(ArmV6::cmp(r0, r1));
	a.emit(ArmV6::bne(f));
	a.emit(ArmV6::adds(r0, 1));
	a.emit(ArmV6::b(g));
	a.label(f);
	a.emit(ArmV6::subs(r1, 2));
	a.label(g);

	a.commit();

	CHECK(code[0] == 0x4288); // cmp r0, r1
	CHECK(code[1] == 0xd101); // bne.n 8 <f>
	CHECK(code[2] == 0x3001); // adds r0, #1
	CHECK(code[3] == 0xe000); // b.n a <g>
	CHECK(code[4] == 0x3902); // subs r1, #2
}

TEST(Asm, Loop)
{
	uint16_t code[4], *labels[2];
	Assembler a(code, sizeof(code) / sizeof(code[0]), labels, sizeof(labels) / sizeof(labels[0]));

	Assembler::Label f(0), g(1);
	a.label(f);
	a.emit(ArmV6::cmp(r0, r1));
	a.emit(ArmV6::beq(g));
	a.emit(ArmV6::adds(r0, 1));
	a.emit(ArmV6::b(f));
	a.label(g);

	a.commit();

	CHECK(code[0] == 0x4288); // cmp r0, r1
	CHECK(code[1] == 0xd001); // beq.n 8 <g>
	CHECK(code[2] == 0x3001); // adds r0, #1
	CHECK(code[3] == 0xe7fb); // b.n 0 <f>
}
