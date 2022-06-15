#ifndef JIT_ARMV6_H_
#define JIT_ARMV6_H_

#include <cstdint>
#include <cassert>

struct ArmV6
{
	using Reg = uint8_t;

private:
	static constexpr inline uint16_t isnReg2(uint16_t op, Reg dn, uint16_t m)
	{
		assert(dn < 8 && m < 8);
		return (op << 6) (m << 3) | dn;
	}

	static constexpr inline uint16_t isnReg3(uint16_t op, Reg d, Reg n, uint16_t m)
	{
		assert(d < 8 && m < 8 && n < 32);
		return (op << 9) | (m << 6) | (n << 3) | d;
	}

	static constexpr inline uint16_t isnImm5(uint16_t op, Reg dnt, Reg m, uint16_t imm5)
	{
		assert(dnt < 8 && m < 8 && imm5 < 32);
		return (op << 11) | (imm5 << 6) | (m << 3) | dnt;
	}

	static constexpr inline uint16_t isnImm8(uint16_t op, Reg r, uint16_t imm8)
	{
		assert(r < 8 && imm8 < 256);
		return (op << 11) | (r << 8) | imm8;
	}

	static constexpr inline uint16_t isnImm7(uint16_t op, uint16_t imm7)
	{
		assert(imm7 < 128);
		return (op << 7) | imm7;
	}

	static constexpr inline uint16_t isnHiReg(uint16_t op, Reg dn, Reg m)
	{
		assert(dn < 16 && m < 16);
		return (op << 8) | (((uint16_t)dn >> 3) << 7) | m << 3 | (dn & 0b0111);
	}

	enum class DpRegOp: uint16_t
	{
		AND = 0b0000,
		EOR = 0b0001,
		LSL = 0b0010,
		LSR = 0b0011,
		ASR = 0b0100,
		ADC = 0b0101,
		SBC = 0b0110,
		ROR = 0b0111,
		TST = 0b1000,
		RSB = 0b1001,
		CMP = 0b1010,
		CMN = 0b1011,
		ORR = 0b1100,
		MUL = 0b1101,
		BIC = 0b1110,
		MVN = 0b0001
	};

	static constexpr inline uint16_t regDataProcessing(DpRegOp op, Reg dn, Reg m) {
		return isnReg2(0b010000'0000 | (uint16_t)op, dn, m);
	}

	enum class LsRegOp: uint16_t
	{
		STR   = 0b000,
		STRH  = 0b001,
		STRB  = 0b010,
		LDRSB = 0b011,
		LDR   = 0b110,
		LDRH  = 0b101,
		LDRB  = 0b110,
		LDRSH = 0b111,
	};

	static constexpr inline uint16_t lsReg(LsRegOp op, Reg t, Reg n, Reg m) {
		return isnReg3(0b0101'000' | (uint16_t)op , t, n, m);
	}

	static constexpr inline uint16_t addSub3(bool isSub, bool immArg3, Reg d, Reg n, uint16_t arg3) {
		return isnReg3(0b000'11'0'0 | ((uint16_t)immArg3 << 1) | ((uint16_t)isSub), d, n, arg3);
	}

	enum class ShImmOp: uint16_t
	{
		LSL = 0b00,
		LSR = 0b01,
		ASR = 0b10,
	};

	static constexpr inline uint16_t shiftImm5(ShImmOp op, Reg d, Reg m, uint16_t imm5) {
		return isnImm5(0b000'00 | (uint16_t)op, d, m, imm5);
	}

	enum class LsImmOp: uint16_t
	{
		STR  = 0b01100,
		LDR  = 0b01101,
		STRB = 0b01110,
		LDRB = 0b01111,
		STRH = 0b10000,
		LDRH = 0b10001,
	};

	static constexpr inline uint16_t lsImm5(LsImmOp op, Reg t, Reg n, uint16_t imm5) {
		return isnImm5((uint16_t)op, t, n, imm5);
	}

	enum class DpImmOp: uint16_t
	{
		MOV = 0b00,
		CMP = 0b01,
		ADD = 0b10,
		SUB = 0b11,
	};

	static constexpr inline uint16_t dpImm8(DpImmOp op, Reg r, uint16_t imm8) {
		return isnImm8(0b001'00 | (uint16_t)op, r, imm8);
	}

	static constexpr inline uint16_t lsSpImm8(bool isLoad, Reg t, uint16_t imm8) {
		return isnImm8(0b1001'0 | ((uint16_t)isLoad), t, imm8);
	}

	static constexpr inline uint16_t ldrImm8(Reg t, uint16_t imm8) {
		return isnImm8(0b01001, t, imm8);
	}

	static constexpr inline uint16_t adrImm8(Reg t, uint16_t imm8) {
		return isnImm8(0b10100, t, imm8);
	}

	static constexpr inline uint16_t addSpImm8(Reg t, uint16_t imm8) {
		return isnImm8(0b10101, t, imm8);
	}

	static constexpr inline uint16_t incrSpImm7(uint16_t imm7) {
		return isnImm7(0b1011'0000'0, imm7);
	}

	static constexpr inline uint16_t decrSpImm7(uint16_t imm7) {
		return isnImm7(0b1011'0000'1, imm7);
	}

	static constexpr inline uint16_t extend(bool isUnsigned, bool isHalfword, Reg d, Reg m) {
		return isnReg2(0b1011'001'00 | ((uint16_t)isUnsigned << 1)| (uint16_t)isHalfword, d, m);
	}

	////////////////////////////
	//                        //
	//       A5-89 push       //
	//                        //
	////////////////////////////

	enum class HiRegOp: uint16_t
	{
		ADD = 0b00,
		CMP = 0b01,
		MOV = 0b10,
		JMP = 0b11,
	};

	static constexpr inline uint16_t hiReg(HiRegOp op, Reg dn, Reg m) {
		return isnHiReg(0b010001'00 | (uint16_t)op, dn, m);
	}

public:
	static constexpr inline uint16_t ands(Reg dn, Reg m) { return regDataProcessing(DpRegOp::AND, dn, m); }
	static constexpr inline uint16_t eors(Reg dn, Reg m) { return regDataProcessing(DpRegOp::EOR, dn, m); }
	static constexpr inline uint16_t lsls(Reg dn, Reg m) { return regDataProcessing(DpRegOp::LSL, dn, m); }
	static constexpr inline uint16_t lsrs(Reg dn, Reg m) { return regDataProcessing(DpRegOp::LSR, dn, m); }
	static constexpr inline uint16_t asrs(Reg dn, Reg m) { return regDataProcessing(DpRegOp::ASR, dn, m); }
	static constexpr inline uint16_t adcs(Reg dn, Reg m) { return regDataProcessing(DpRegOp::ADC, dn, m); }
	static constexpr inline uint16_t sbcs(Reg dn, Reg m) { return regDataProcessing(DpRegOp::SBC, dn, m); }
	static constexpr inline uint16_t rors(Reg dn, Reg m) { return regDataProcessing(DpRegOp::ROR, dn, m); }
	static constexpr inline uint16_t tst(Reg n, Reg m)   { return regDataProcessing(DpRegOp::TST, n, m); }
	static constexpr inline uint16_t rsb(Reg d, Reg m)   { return regDataProcessing(DpRegOp::RSB, d, m); }
	static constexpr inline uint16_t cmp(Reg n, Reg m)   { return regDataProcessing(DpRegOp::CMP, n, m); }
	static constexpr inline uint16_t cmn(Reg n, Reg m)   { return regDataProcessing(DpRegOp::CMN, n, m); }
	static constexpr inline uint16_t orrs(Reg dn, Reg m) { return regDataProcessing(DpRegOp::ORR, dn, m); }
	static constexpr inline uint16_t muls(Reg dn, Reg m) { return regDataProcessing(DpRegOp::MUL, dn, m); }
	static constexpr inline uint16_t bics(Reg dn, Reg m) { return regDataProcessing(DpRegOp::BIC, dn, m); }
	static constexpr inline uint16_t mvns(Reg dn, Reg m) { return regDataProcessing(DpRegOp::MVN, dn, m); }

	static constexpr inline uint16_t lslsImm5(Reg d, Reg m, uint16_t imm5) { return shiftImm5(ShImmOp::LSL, d, m, imm5); }
	static constexpr inline uint16_t lsrsImm5(Reg d, Reg m, uint16_t imm5) { return shiftImm5(ShImmOp::LSR, d, m, imm5); }
	static constexpr inline uint16_t asrsImm5(Reg d, Reg m, uint16_t imm5) { return shiftImm5(ShImmOp::ASR, d, m, imm5); }

	static constexpr inline uint16_t addsReg(Reg d, Reg n, Reg m)          { return addSub3(false, false, d, n, m); }
	static constexpr inline uint16_t addsImm3(Reg d, Reg n, uint16_t imm3) { return addSub3(false, true,  d, n, imm3); }
	static constexpr inline uint16_t subsReg(Reg d, Reg n, Reg m)          { return addSub3(true,  false, d, n, m); }
	static constexpr inline uint16_t subsImm3(Reg d, Reg n, uint16_t imm3) { return addSub3(true,  true,  d, n, imm3); }

	static constexpr inline uint16_t movsImm8(Reg d, uint16_t imm8) { return dpImm8(DpImmOp::MOV, d, imm8); }
	static constexpr inline uint16_t cmpImm8(Reg n, uint16_t imm8)  { return dpImm8(DpImmOp::CMP, n, imm8); }
	static constexpr inline uint16_t addsImm8(Reg dn, uint16_t imm8) { return dpImm8(DpImmOp::ADD, dn, imm8); }
	static constexpr inline uint16_t subsImm8(Reg dn, uint16_t imm8) { return dpImm8(DpImmOp::SUB, dn, imm8); }

	static constexpr inline uint16_t addHi(Reg dn, Reg m) { return hiReg(HiRegOp::ADD, dn, m); }
	static constexpr inline uint16_t movHi(Reg dn, Reg m) { return hiReg(HiRegOp::MOV, dn, m); }
	static constexpr inline uint16_t blx(Reg m)           { return hiReg(HiRegOp::JMP, 0b1000, m); }
	static constexpr inline uint16_t bx(Reg m)            { return hiReg(HiRegOp::JMP, 0b0000, m); }
	static constexpr inline uint16_t cmpHi(Reg n, Reg m)
	{
		assert((n & 0b1000) || (m & 0b1000));
		return hiReg(HiRegOp::CMP, n, m);
	}

	static constexpr inline uint16_t strReg(Reg t, Reg n, Reg m)   { return lsReg(LsRegOp::STR, t, n, m); }
	static constexpr inline uint16_t strhReg(Reg t, Reg n, Reg m)  { return lsReg(LsRegOp::STRH, t, n, m); }
	static constexpr inline uint16_t strbReg(Reg t, Reg n, Reg m)  { return lsReg(LsRegOp::STRB, t, n, m); }
	static constexpr inline uint16_t ldrsbReg(Reg t, Reg n, Reg m) { return lsReg(LsRegOp::LDRSB, t, n, m); }
	static constexpr inline uint16_t ldrReg(Reg t, Reg n, Reg m)   { return lsReg(LsRegOp::LDR, t, n, m); }
	static constexpr inline uint16_t ldrhReg(Reg t, Reg n, Reg m)  { return lsReg(LsRegOp::LDRH, t, n, m); }
	static constexpr inline uint16_t ldrbReg(Reg t, Reg n, Reg m)  { return lsReg(LsRegOp::LDRB, t, n, m); }
	static constexpr inline uint16_t ldrshReg(Reg t, Reg n, Reg m) { return lsReg(LsRegOp::LDRSH, t, n, m); }

	static constexpr inline uint16_t strImm5(Reg t, Reg n, uint16_t imm5)  { return lsImm5(LsImmOp::STR, t, n, imm5); }
	static constexpr inline uint16_t ldrImm5(Reg t, Reg n, uint16_t imm5)  { return lsImm5(LsImmOp::LDR, t, n, imm5); }
	static constexpr inline uint16_t strbImm5(Reg t, Reg n, uint16_t imm5) { return lsImm5(LsImmOp::STRB, t, n, imm5); }
	static constexpr inline uint16_t ldrbImm5(Reg t, Reg n, uint16_t imm5) { return lsImm5(LsImmOp::LDRB, t, n, imm5); }
	static constexpr inline uint16_t strhImm5(Reg t, Reg n, uint16_t imm5) { return lsImm5(LsImmOp::STRH, t, n, imm5); }
	static constexpr inline uint16_t ldrhImm5(Reg t, Reg n, uint16_t imm5) { return lsImm5(LsImmOp::LDRH, t, n, imm5); }

	static constexpr inline uint16_t strSpImm8(Reg t, uint16_t imm8) { return lsSpImm8(false, t, imm8); }
	static constexpr inline uint16_t ldrSpImm8(Reg t, uint16_t imm8) { return lsSpImm8(true, t, imm8); }

};

#endif /* JIT_ARMV6_H_ */
