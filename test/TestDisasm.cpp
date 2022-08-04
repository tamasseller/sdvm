#include "TestUtils.h"

#include "Assembler.h"
#include "Disassembler.h"

TEST_GROUP(DisAsm)
{
	static std::string single(uint16_t arg)
	{
		uint16_t isn alignas(uint32_t);
		isn = arg;

		auto ret = Disassembler::disassemble(&isn, &isn + 1);
		CHECK(ret.size() == 1);
		return ret[0];
	}
};

TEST(DisAsm, Reg2)
{
	CHECK(single(0x4008) == "ands r0, r1");
	CHECK(single(0x4051) == "eors r1, r2");
	CHECK(single(0x409a) == "lsls r2, r3");
	CHECK(single(0x40e3) == "lsrs r3, r4");
	CHECK(single(0x412c) == "asrs r4, r5");
	CHECK(single(0x4175) == "adcs r5, r6");
	CHECK(single(0x4187) == "sbcs r7, r0");
	CHECK(single(0x41c8) == "rors r0, r1");
	CHECK(single(0x4211) == "tst r1, r2");
	CHECK(single(0x425a) == "negs r2, r3");
	CHECK(single(0x42a3) == "cmp r3, r4");
	CHECK(single(0x42ec) == "cmn r4, r5");
	CHECK(single(0x4335) == "orrs r5, r6");
	CHECK(single(0x437e) == "muls r6, r7");
	CHECK(single(0x4387) == "bics r7, r0");
	CHECK(single(0x43c8) == "mvns r0, r1");
	CHECK(single(0xb211) == "sxth r1, r2");
	CHECK(single(0xb25a) == "sxtb r2, r3");
	CHECK(single(0xb2a3) == "uxth r3, r4");
	CHECK(single(0xb2ec) == "uxtb r4, r5");
	CHECK(single(0xba35) == "rev r5, r6");
	CHECK(single(0xba7e) == "rev16 r6, r7");
	CHECK(single(0xbac7) == "revsh r7, r0");
}

TEST(DisAsm, Reg3)
{
	CHECK(single(0x1888) == "adds r0, r1, r2");
	CHECK(single(0x1ad1) == "subs r1, r2, r3");
	CHECK(single(0x1d1a) == "adds r2, r3, #4");
	CHECK(single(0x1f63) == "subs r3, r4, #5");
	CHECK(single(0x51ac) == "str r4, [r5, r6]");
	CHECK(single(0x53f5) == "strh r5, [r6, r7]");
	CHECK(single(0x543e) == "strb r6, [r7, r0]");
	CHECK(single(0x5647) == "ldrsb r7, [r0, r1]");
	CHECK(single(0x5888) == "ldr r0, [r1, r2]");
	CHECK(single(0x5ad1) == "ldrh r1, [r2, r3]");
	CHECK(single(0x5d1a) == "ldrb r2, [r3, r4]");
	CHECK(single(0x5f63) == "ldrsh r3, [r4, r5]");
}

TEST(DisAsm, Imm5)
{
	CHECK(single(0x0088) == "lsls r0, r1, #2");
	CHECK(single(0x08d1) == "lsrs r1, r2, #3");
	CHECK(single(0x111a) == "asrs r2, r3, #4");
	CHECK(single(0x6163) == "str r3, [r4, #20]");
	CHECK(single(0x69ac) == "ldr r4, [r5, #24]");
	CHECK(single(0x71f5) == "strb r5, [r6, #7]");
	CHECK(single(0x7a3e) == "ldrb r6, [r7, #8]");
	CHECK(single(0x8247) == "strh r7, [r0, #18]");
	CHECK(single(0x8a88) == "ldrh r0, [r1, #20]");
}

TEST(DisAsm, Imm7)
{
	CHECK(single(0xb069) == "add sp, #420");
	CHECK(single(0xb0bc) == "sub sp, #240");
}

TEST(DisAsm, Imm8)
{
	CHECK(single(0x2022) == "movs r0, #34");
	CHECK(single(0x292d) == "cmp r1, #45");
	CHECK(single(0x3238) == "adds r2, #56");
	CHECK(single(0x3b43) == "subs r3, #67");
	CHECK(single(0x944e) == "str r4, [sp, #312]");
	CHECK(single(0x9d59) == "ldr r5, [sp, #356]");
	CHECK(single(0x4e19) == "ldr r6, [pc, #100]");
	CHECK(single(0xa77b) == "add r7, pc, #492");
	CHECK(single(0xa8ea) == "add r0, sp, #936");
}

TEST(DisAsm, NoArg)
{
	CHECK(single(0xb662) == "cpsie i");
	CHECK(single(0xb672) == "cpsid i");
	CHECK(single(0xbf00) == "nop");
	CHECK(single(0xbf10) == "yield");
	CHECK(single(0xbf20) == "wfe");
	CHECK(single(0xbf30) == "wfi");
	CHECK(single(0xbf40) == "sev");
}

TEST(DisAsm, CondSvcUdf)
{
	CHECK(single(0xd0fe) == "beq -4");
	CHECK(single(0xd10b) == "bne 22");
	CHECK(single(0xd2fc) == "bhs -8");
	CHECK(single(0xd309) == "blo 18");
	CHECK(single(0xd4fa) == "bmi -12");
	CHECK(single(0xd507) == "bpl 14");
	CHECK(single(0xd6f8) == "bvs -16");
	CHECK(single(0xd705) == "bvc 10");
	CHECK(single(0xd8f6) == "bhi -20");
	CHECK(single(0xd903) == "bls 6");
	CHECK(single(0xdaf4) == "bge -24");
	CHECK(single(0xdb01) == "blt 2");
	CHECK(single(0xdcf2) == "bgt -28");
	CHECK(single(0xddff) == "ble -2");
	CHECK(single(0xdedf) == "udf #223");
	CHECK(single(0xdf4c) == "svc #76");
}

TEST(DisAsm, HiReg)
{
	CHECK(single(0x44d1) == "add r9, r10");
	CHECK(single(0x45da) == "cmp r10, r11");
	CHECK(single(0x46e3) == "mov r11, r12");
	CHECK(single(0x47c0) == "blx r8");
	CHECK(single(0x4770) == "bx lr");
}

TEST(DisAsm, PushPop)
{
	CHECK(single(0xb455) == "push {r0, r2, r4, r6}");
	CHECK(single(0xbc2a) == "pop {r1, r3, r5}");
	CHECK(single(0xb580) == "push {r7, lr}");
	CHECK(single(0xbd00) == "pop {pc}");
}

TEST(DisAsm, lsMia)
{
	CHECK(single(0xc955) == "ldmia r1!, {r0, r2, r4, r6}");
	CHECK(single(0xc855) == "ldmia r0, {r0, r2, r4, r6}");
	CHECK(single(0xc22a) == "stmia r2!, {r1, r3, r5}");
}

TEST(DisAsm, Bkpt)
{
	CHECK(single(0xbeab) == "bkpt #171");
}

TEST(DisAsm, Branch)
{
	CHECK(single(0xe002) == "b 4"),
	CHECK(single(0xe7fb) == "b -10");
}

TEST(DisAsm, VmTab)
{
	uint16_t code[2] alignas(uint32_t);
	Assembler a(code, 2, nullptr, 0);
	a.vmTab(0x1234);
	auto end = a.assemble();
	CHECK(end == code + 2);

	auto res = Disassembler::disassemble(code, code + 2);
	CHECK(res.size() == 2);
	CHECK(res[0] == "blx r9");
	CHECK(res[1] == ".short 0x1234");
}

TEST(DisAsm, Literal)
{
	uint16_t code[6] alignas(uint32_t);
	Assembler a(code, 6, nullptr, 0);
	a.emit(ArmV6::ldrPc(ArmV6::LoReg(3), a.literal(0x13337)));
	a.emit(ArmV6::addPc(ArmV6::LoReg(7), a.literal(0xc007c0de)));
	auto end = a.assemble();
	CHECK(end == code + 6);

	auto res = Disassembler::disassemble(code, end);
	CHECK(res.size() == 4);

	CHECK(res[0] == "ldr r3, [pc, #0]");
	CHECK(res[1] == "add r7, pc, #4");
	CHECK(res[2] == ".long 0x00013337");
	CHECK(res[3] == ".long 0xc007c0de");
}
