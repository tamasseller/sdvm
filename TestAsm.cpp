#include "TestUtils.h"

#include "jit/armv6.h"

#include <iostream>

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

TEST(Asm, Reg2)
{
	CHECK(ArmV6::adcs (r7, r0) == 0x4147); // adcs  r7, r0
	CHECK(ArmV6::ands (r0, r1) == 0x4008); // ands  r0, r1
	CHECK(ArmV6::eors (r1, r2) == 0x4051); // eors  r1, r2
	CHECK(ArmV6::lsls (r2, r3) == 0x409a); // lsls  r2, r3
	CHECK(ArmV6::lsrs (r3, r4) == 0x40e3); // lsrs  r3, r4
	CHECK(ArmV6::asrs (r4, r5) == 0x412c); // asrs  r4, r5
	CHECK(ArmV6::adcs (r5, r6) == 0x4175); // adcs  r5, r6
	CHECK(ArmV6::sbcs (r6, r7) == 0x41be); // sbcs  r6, r7
	CHECK(ArmV6::rors (r7, r0) == 0x41c7); // rors  r7, r0
	CHECK(ArmV6::tst  (r0, r1) == 0x4208); // tst   r0, r1
	CHECK(ArmV6::negs (r1, r2) == 0x4251); // negs  r1, r2
	CHECK(ArmV6::cmp  (r2, r3) == 0x429a); // cmp   r2, r3
	CHECK(ArmV6::cmn  (r3, r4) == 0x42e3); // cmn   r3, r4
	CHECK(ArmV6::orrs (r4, r5) == 0x432c); // orrs  r4, r5
	CHECK(ArmV6::muls (r5, r6) == 0x4375); // muls  r5, r6
	CHECK(ArmV6::bics (r6, r7) == 0x43be); // bics  r6, r7
	CHECK(ArmV6::mvns (r7, r0) == 0x43c7); // mvns  r7, r0
	CHECK(ArmV6::sxth (r0, r1) == 0xb208); // sxth  r0, r1
	CHECK(ArmV6::sxtb (r1, r2) == 0xb251); // sxtb  r1, r2
	CHECK(ArmV6::uxth (r2, r3) == 0xb29a); // uxth  r2, r3
	CHECK(ArmV6::uxtb (r3, r4) == 0xb2e3); // uxtb  r3, r4
	CHECK(ArmV6::rev  (r4, r5) == 0xba2c); // rev   r4, r5
	CHECK(ArmV6::rev16(r5, r6) == 0xba75); // rev16 r5, r6
	CHECK(ArmV6::revsh(r6, r7) == 0xbafe); // revsh r6, r7
}

TEST(Asm, Reg3)
{
	CHECK(ArmV6::adds (r0, r1, ArmV6::Imm<3>(2)) == 0x1c88); // adds  r0, r1, #2
	CHECK(ArmV6::subs (r1, r2, ArmV6::Imm<3>(3)) == 0x1ed1); // subs  r1, r2, #3
	CHECK(ArmV6::adds (r2, r3, r4)               == 0x191a); // adds  r2, r3, r4
	CHECK(ArmV6::subs (r3, r4, r5)               == 0x1b63); // subs  r3, r4, r5
	CHECK(ArmV6::str  (r4, r5, r6)               == 0x51ac); // str   r4, [r5, r6]
	CHECK(ArmV6::strh (r5, r6, r7)               == 0x53f5); // strh  r5, [r6, r7]
	CHECK(ArmV6::strb (r6, r7, r0)               == 0x543e); // strb  r6, [r7, r0]
	CHECK(ArmV6::ldrsb(r7, r0, r1)               == 0x5647); // ldrsb r7, [r0, r1]
	CHECK(ArmV6::ldr  (r0, r1, r2)               == 0x5888); // ldr   r0, [r1, r2]
	CHECK(ArmV6::ldrh (r1, r2, r3)               == 0x5ad1); // ldrh  r1, [r2, r3]
	CHECK(ArmV6::ldrb (r2, r3, r4)               == 0x5d1a); // ldrb  r2, [r3, r4]
	CHECK(ArmV6::ldrsh(r3, r4, r5)               == 0x5f63); // ldrsh r3, [r4, r5]
}

TEST(Asm, Imm5)
{
	CHECK(ArmV6::lsls(r0, r1, 31)                    == 0x07c8); // lsls r0,  r1, #31
	CHECK(ArmV6::lsrs(r1, r2, 30)                    == 0x0f91); // lsrs r1,  r2, #30
	CHECK(ArmV6::asrs(r2, r3, 29)                    == 0x175a); // asrs r2,  r3, #29
	CHECK(ArmV6::strb(r3, r4, ArmV6::Uoff<0, 5>(28)) == 0x7723); // strb r3, [r4, #28]
	CHECK(ArmV6::ldrb(r4, r5, ArmV6::Uoff<0, 5>(27)) == 0x7eec); // ldrb r4, [r5, #27]
	CHECK(ArmV6::strh(r5, r6, ArmV6::Uoff<1, 5>(52)) == 0x86b5); // strh r5, [r6, #52]
	CHECK(ArmV6::ldrh(r6, r7, ArmV6::Uoff<1, 5>(50)) == 0x8e7e); // ldrh r6, [r7, #50]
	CHECK(ArmV6::str (r7, r0, ArmV6::Uoff<2, 5>(96)) == 0x6607); // str  r7, [r0, #96]
	CHECK(ArmV6::ldr (r0, r1, ArmV6::Uoff<2, 5>(92)) == 0x6dc8); // ldr  r0, [r1, #92]
}

TEST(Asm, Imm7)
{
	CHECK(ArmV6::incrSp(508) == 0xb07f); // add sp, #508
	CHECK(ArmV6::decrSp(4)   == 0xb081); // sub sp, #4
}

TEST(Asm, Imm8)
{
	CHECK(ArmV6::movs (r0, 130)                    == 0x2082); // movs r0, #130
	CHECK(ArmV6::cmp  (r1, ArmV6::Imm<8>(140))     == 0x298c); // cmp  r1, #140
	CHECK(ArmV6::adds (r2, ArmV6::Imm<8>(150))     == 0x3296); // adds r2, #150
	CHECK(ArmV6::subs (r3, ArmV6::Imm<8>(160))     == 0x3ba0); // subs r3, #160
	CHECK(ArmV6::strSp(r4, ArmV6::Uoff<2, 8>(680)) == 0x94aa); // str  r4, [sp, #680]
	CHECK(ArmV6::ldrSp(r5, ArmV6::Uoff<2, 8>(720)) == 0x9db4); // ldr  r5, [sp, #720]
	CHECK(ArmV6::ldrPc(r6, ArmV6::Uoff<2, 8>(760)) == 0x4ebe); // ldr  r6, [pc, #760]
	CHECK(ArmV6::adr  (r7, ArmV6::Uoff<2, 8>(800)) == 0xa7c8); // add  r7, pc, #800
	CHECK(ArmV6::addSp(r0, ArmV6::Uoff<2, 8>(840)) == 0xa8d2); // add  r0, sp, #840
}

TEST(Asm, HiReg)
{
	CHECK(ArmV6::add (r8, r9)   == 0x44c8); // add r8, r9
	CHECK(ArmV6::mov (r9, r10)  == 0x46d1); // mov r9, sl
	CHECK(ArmV6::cmp (r10, r11) == 0x45da); // cmp sl, fp
	CHECK(ArmV6::blx (r11)      == 0x47d8); // blx fp
	CHECK(ArmV6::bx  (r12)      == 0x4760); // bx  ip
}

TEST(Asm, MultiLoadStore)
{
	CHECK(ArmV6::pushWoLr  (ArmV6::LoRegs{}.add(r0))                     == 0xb401); // push {r0}
	CHECK(ArmV6::pushWithLr(ArmV6::LoRegs{}.add(r1))                     == 0xb502); // push {r1, lr}
	CHECK(ArmV6::popWoPc   (ArmV6::LoRegs{}.add(r2).add(r3))             == 0xbc0c); // pop  {r2, r3}
	CHECK(ArmV6::popWithPc (ArmV6::LoRegs{}.add(r4).add(r5))             == 0xbd30); // pop  {r4, r5, pc}
	CHECK(ArmV6::stmia     (r6, ArmV6::LoRegs{}.add(r7))                 == 0xc680); // stmia r6!, {r7}
	CHECK(ArmV6::ldmia     (r0, ArmV6::LoRegs{}.add(r1).add(r2))         == 0xc806); // ldmia r0!, {r1, r2}
	CHECK(ArmV6::ldmia     (r3, ArmV6::LoRegs{}.add(r3).add(r4).add(r5)) == 0xcb38); // ldmia r3, {r3, r4, r5}
}

TEST(Asm, Branch)
{
	CHECK(ArmV6::beq(ArmV6::Ioff<1, 8>(30)) == 0xd00f);
	CHECK(ArmV6::bne(ArmV6::Ioff<1, 8>(28)) == 0xd10e);
	CHECK(ArmV6::bhs(ArmV6::Ioff<1, 8>(26)) == 0xd20d);
	CHECK(ArmV6::blo(ArmV6::Ioff<1, 8>(24)) == 0xd30c);
	CHECK(ArmV6::bmi(ArmV6::Ioff<1, 8>(22)) == 0xd40b);
	CHECK(ArmV6::bpl(ArmV6::Ioff<1, 8>(20)) == 0xd50a);
	CHECK(ArmV6::bvs(ArmV6::Ioff<1, 8>(18)) == 0xd609);
	CHECK(ArmV6::bvc(ArmV6::Ioff<1, 8>(16)) == 0xd708);
	CHECK(ArmV6::bhi(ArmV6::Ioff<1, 8>(14)) == 0xd807);
	CHECK(ArmV6::bls(ArmV6::Ioff<1, 8>(12)) == 0xd906);
	CHECK(ArmV6::bge(ArmV6::Ioff<1, 8>(10)) == 0xda05);
	CHECK(ArmV6::blt(ArmV6::Ioff<1, 8>(8))  == 0xdb04);
	CHECK(ArmV6::bgt(ArmV6::Ioff<1, 8>(6))  == 0xdc03);
	CHECK(ArmV6::ble(ArmV6::Ioff<1, 8>(4))  == 0xdd02);
	CHECK(ArmV6::b  (ArmV6::Ioff<1, 11>(2)) == 0xe001);
	CHECK(ArmV6::udf(ArmV6::Imm<8>(1))      == 0xde01);
	CHECK(ArmV6::svc(ArmV6::Imm<8>(0))      == 0xdf00);
}

TEST(Asm, Other)
{
	CHECK(ArmV6::bkpt(1) == 0xbe01);
	CHECK(ArmV6::cpsie() == 0xb662);
	CHECK(ArmV6::cpsid() == 0xb672);
	CHECK(ArmV6::nop()   == 0xbf00);
	CHECK(ArmV6::yield() == 0xbf10);
	CHECK(ArmV6::wfe()   == 0xbf20);
	CHECK(ArmV6::wfi()   == 0xbf30);
	CHECK(ArmV6::sev()   == 0xbf40);
}
