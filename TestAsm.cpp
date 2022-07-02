#include "TestUtils.h"

#include "jit/Assembler.h"

TEST_GROUP(Asm)
{
	ArmV6::LoReg r0, r1, r2, r3, r4, r5, r6, r7;
	ArmV6::AnyReg r8, r9, r10, r11, r12, sp, lr, pc;

	TEST_SETUP()
	{
		r0  = ArmV6::LoReg(0);
		r1  = ArmV6::LoReg(1);
		r2  = ArmV6::LoReg(2);
		r3  = ArmV6::LoReg(3);
		r4  = ArmV6::LoReg(4);
		r5  = ArmV6::LoReg(5);
		r6  = ArmV6::LoReg(6);
		r7  = ArmV6::LoReg(7);
		r8  = ArmV6::AnyReg(8);
		r9  = ArmV6::AnyReg(9);
		r10 = ArmV6::AnyReg(10);
		r11 = ArmV6::AnyReg(11);
		r12 = ArmV6::AnyReg(12);
		sp  = ArmV6::AnyReg(13);
		lr  = ArmV6::AnyReg(14);
		pc  = ArmV6::AnyReg(15);
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
	a.assemble();

	CHECK(code[0] == 0x2010); // movs r0, #16
	CHECK(code[1] == 0x4468); // add r0, sp
	CHECK(code[2] == 0xc803); // ldmia r0, {r0, r1}
	CHECK(code[3] == 0x4408); // add r0, r1
	CHECK(code[4] == 0x9004); // str r0, [sp, #16]
}

TEST(Asm, If)
{
	uint16_t code[4] alignas(4);
	Assembler::LabelInfo labels[1];
	Assembler a(code, sizeof(code) / sizeof(code[0]), labels, sizeof(labels) / sizeof(labels[0]));

	Assembler::Label g(0);
	a.emit(ArmV6::cmp(r0, r1));
	a.emit(ArmV6::bne(g));
	a.emit(ArmV6::adds(r0, 1));
	a.pin(g);

	a.assemble();

	CHECK(code[0] == 0x4288); // cmp r0, r1
	CHECK(code[1] == 0xd100); // bne.n 6 <g>
	CHECK(code[2] == 0x3001); // adds r0, #1
}

TEST(Asm, IfElse)
{
	uint16_t code[6] alignas(4);
	Assembler::LabelInfo labels[2];
	Assembler a(code, sizeof(code) / sizeof(code[0]), labels, sizeof(labels) / sizeof(labels[0]));

	Assembler::Label f(0), g(1);
	a.emit(ArmV6::cmp(r0, r1));
	a.emit(ArmV6::bne(f));
	a.emit(ArmV6::adds(r0, 1));
	a.emit(ArmV6::b(g));
	a.pin(f);
	a.emit(ArmV6::subs(r1, 2));
	a.pin(g);

	a.assemble();

	CHECK(code[0] == 0x4288); // cmp r0, r1
	CHECK(code[1] == 0xd101); // bne.n 8 <f>
	CHECK(code[2] == 0x3001); // adds r0, #1
	CHECK(code[3] == 0xe000); // b.n a <g>
	CHECK(code[4] == 0x3902); // subs r1, #2
}

TEST(Asm, Loop)
{
	uint16_t code[6] alignas(4);
	Assembler::LabelInfo labels[2];
	Assembler a(code, sizeof(code) / sizeof(code[0]), labels, sizeof(labels) / sizeof(labels[0]));

	Assembler::Label f(0), g(1);
	a.pin(f);
	a.emit(ArmV6::cmp(r0, r1));
	a.emit(ArmV6::beq(g));
	a.emit(ArmV6::adds(r0, 1));
	a.emit(ArmV6::b(f));
	a.pin(g);

	a.assemble();

	CHECK(code[0] == 0x4288); // cmp r0, r1
	CHECK(code[1] == 0xd001); // beq.n 8 <g>
	CHECK(code[2] == 0x3001); // adds r0, #1
	CHECK(code[3] == 0xe7fb); // b.n 0 <f>
}

TEST(Asm, LongCond)
{
	uint16_t code[1004] alignas(4);
	Assembler::LabelInfo labels[2];
	Assembler a(code, sizeof(code) / sizeof(code[0]), labels, sizeof(labels) / sizeof(labels[0]));

	Assembler::Label f(0), g(1);
	a.pin(f);
	a.emit(ArmV6::bhi(g));

	for(int i = 0; i < 1000; i++)
	{
		a.emit(ArmV6::lsls(r0, r0, 0));
	}

	a.pin(g);
	a.emit(ArmV6::bls(f));

	a.assemble();

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
	uint16_t code[310] alignas(4);
	Assembler::LabelInfo labels[4];
	Assembler a(code, sizeof(code) / sizeof(code[0]), labels, sizeof(labels) / sizeof(labels[0]));

	Assembler::Label l(0), m(1), n(2), o(3);
	a.pin(l);
	a.emit(ArmV6::bhi(m));
	a.emit(ArmV6::beq(n));
	a.pin(m);
	a.emit(ArmV6::b(m));

	for(int i = 0; i < 300; i++)
	{
		a.emit(ArmV6::lsls(r0, r0, 0));
	}

	a.pin(n);
	a.emit(ArmV6::bmi(o));
	a.emit(ArmV6::bls(l));
	a.pin(o);

	a.assemble();

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
	uint16_t code[6] alignas(4);
	Assembler a(code, sizeof(code) / sizeof(code[0]), nullptr, 0);

	a.emit(ArmV6::ldrPc(r0, a.literal(0xb16b00b5)));
	a.assemble();

	CHECK(code[0] == 0x4800); // ldr r0, [pc, #0] ; (4 <a>)
	CHECK(code[1] == 0xbf00); // nop
	CHECK(code[2] == 0x00b5); // .word	0xb16b00b5
	CHECK(code[3] == 0xb16b);
}


TEST(Asm, ManyLiteralsDeduplicate)
{
	uint16_t code[20] alignas(4);
	Assembler a(code, sizeof(code) / sizeof(code[0]), nullptr, 0);

	a.emit(ArmV6::addPc(r0, a.literal(0xb16b00b5)));
	a.emit(ArmV6::ldrPc(r1, a.literal(0x1337c0de)));
	a.emit(ArmV6::ldrPc(r2, a.literal(0xb16b00b5)));
	a.emit(ArmV6::addPc(r3, a.literal(0xb16b00b5)));
	a.emit(ArmV6::addPc(r4, a.literal(0x1337c0de)));
	a.emit(ArmV6::ldrPc(r5, a.literal(0xb16b00b5)));
	a.emit(ArmV6::ldrPc(r6, a.literal(0x1337c0de)));
	a.emit(ArmV6::addPc(r7, a.literal(0x1337c0de)));
	a.assemble();

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

TEST(Asm, MultipleLiteralsNoRoomOdd)
{
	uint16_t code[10] alignas(4);
	Assembler a(code, sizeof(code) / sizeof(code[0]), nullptr, 0);

	a.emit(ArmV6::ldrPc(r0, a.literal(0x1337c0de)));
	a.emit(ArmV6::ldrPc(r1, a.literal(0xb16b00b5)));
	a.emit(ArmV6::addPc(r2, a.literal(0x600df00d)));
	a.assemble();

	CHECK(code[0] == 0x4801); // ldr r0, [pc, #4] ; (8 <a>)
	CHECK(code[1] == 0x4902); // ldr r1, [pc, #8] ; (c <b>)
	CHECK(code[2] == 0xa202); // add r2, pc, #8 ; (adr r2, 10 <c>)
	CHECK(code[3] == 0xbf00); // nop
	CHECK(code[4] == 0xc0de); // .word 0x1337c0de
	CHECK(code[5] == 0x1337); //
	CHECK(code[6] == 0x00b5); // .word 0xb16b00b5
	CHECK(code[7] == 0xb16b); //
	CHECK(code[8] == 0xf00d); // .word 0x600df00d
	CHECK(code[9] == 0x600d); //
}

TEST(Asm, MultipleLiteralsNoRoomEven)
{
	uint16_t code[12] alignas(4);
	Assembler a(code, sizeof(code) / sizeof(code[0]), nullptr, 0);

	a.emit(ArmV6::ldrPc(r0, a.literal(0x1337c0de)));
	a.emit(ArmV6::ldrPc(r1, a.literal(0xb16b00b5)));
	a.emit(ArmV6::addPc(r2, a.literal(0x600df00d)));
	a.emit(ArmV6::addPc(r3, a.literal(0xfeedbabe)));
	a.assemble();

	CHECK(code[0]  == 0x4801); // ldr r0, [pc, #4] ; (8 <a>)
	CHECK(code[1]  == 0x4902); // ldr r1, [pc, #8] ; (c <b>)
	CHECK(code[2]  == 0xa202); // add r2, pc, #8 ; (adr r2, 10 <c>)
	CHECK(code[3]  == 0xa303); // add r3, pc, #12 ; (adr r3, 14 <d>)
	CHECK(code[4]  == 0xc0de); // .word 0x1337c0de
	CHECK(code[5]  == 0x1337); //
	CHECK(code[6]  == 0x00b5); // .word 0xb16b00b5
	CHECK(code[7]  == 0xb16b); //
	CHECK(code[8]  == 0xf00d); // .word 0x600df00d
	CHECK(code[9]  == 0x600d); //
	CHECK(code[10] == 0xbabe); // .word 0cfeedbabe
	CHECK(code[11] == 0xfeed); //
}

TEST(Asm, InlineLiteralAgainstBranchAndLiteral)
{
	uint16_t code[14] alignas(4);
	Assembler::LabelInfo labels[2];
	Assembler a(code, sizeof(code) / sizeof(code[0]), labels, sizeof(labels) / sizeof(labels[0]));

	Assembler::Label f(0), g(1);
	a.emit(ArmV6::b(g));
	a.pin(f);
	
	a.emit(ArmV6::ldrPc(ArmV6::LoReg(0), a.literal(0x1337f00du)));
	a.vmTab(0x0001);
	a.vmTab(0xb800);
	a.emit(ArmV6::adds(ArmV6::LoReg(0), ArmV6::LoReg(1), ArmV6::LoReg(2)));
	a.vmTab(0x4800);

	a.pin(g);
	a.emit(ArmV6::bls(f));

	a.assemble();

	CHECK(code[0]  == 0xe007); // b.n 12 <g>
	CHECK(code[1]  == 0x4804); // ldr r0, [pc, #16]	; (14 <l>)
	CHECK(code[2]  == 0x47c8); // blx r9
	CHECK(code[3]  == 0x0001); // .short 0x0001
	CHECK(code[4]  == 0x47c8); // blx r9
	CHECK(code[5]  == 0xb800); // .short 0xb800
	CHECK(code[6]  == 0x1888); // adds r0, r1, r2
	CHECK(code[7]  == 0x47c8); // blx r9
	CHECK(code[8]  == 0x4800); // .short 0x4800
	CHECK(code[9]  == 0xd9f6); // bls.n 2 <f>
	CHECK(code[10] == 0xf00d); // .word 0x1337f00d
	CHECK(code[11] == 0x1337);
}

TEST(Asm, StrayNops)
{
	uint16_t code[10] alignas(4);
	Assembler::LabelInfo labels[1];
	Assembler a(code, sizeof(code) / sizeof(code[0]), labels, sizeof(labels) / sizeof(labels[0]));

	Assembler::Label g(0);
	a.emit(ArmV6::nop());
	a.emit(ArmV6::cmp(r0, r1));
	a.emit(ArmV6::nop());
	a.emit(ArmV6::bne(g));
	a.emit(ArmV6::nop());
	a.emit(ArmV6::ldrPc(r0, a.literal(0x12345678u)));
	a.pin(g);
	a.emit(ArmV6::nop());

	CHECK(a.assemble() - code == 6);

	CHECK(code[0] == 0x4288); // cmp r0, r1
	CHECK(code[1] == 0xd100); // bne.n 6 <g>
	CHECK(code[2] == 0x4800); // adds r0, #1
	CHECK(code[3] == 0xbf00);
	CHECK(code[4] == 0x5678);
	CHECK(code[5] == 0x1234);
}
