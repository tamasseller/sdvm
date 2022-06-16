#ifndef JIT_ARMV6_H_
#define JIT_ARMV6_H_

#include <cstdint>
#include <cassert>

struct ArmV6
{
	using Reg = uint8_t;

private:
	enum class Reg2Op: uint16_t
	{
		//            opcode  m   dn
		AND   = 0b0100000000'000'000,
		EOR   = 0b0100000001'000'000,
		LSL   = 0b0100000010'000'000,
		LSR   = 0b0100000011'000'000,
		ASR   = 0b0100000100'000'000,
		ADC   = 0b0100000101'000'000,
		SBC   = 0b0100000110'000'000,
		ROR   = 0b0100000111'000'000,
		TST   = 0b0100001000'000'000,
		RSB   = 0b0100001001'000'000,
		CMP   = 0b0100001010'000'000,
		CMN   = 0b0100001011'000'000,
		ORR   = 0b0100001100'000'000,
		MUL   = 0b0100001101'000'000,
		BIC   = 0b0100001110'000'000,
		MVN   = 0b0100001111'000'000,
		SXH   = 0b1011001000'000'000,
		SXB   = 0b1011001001'000'000,
		UXH   = 0b1011001010'000'000,
		UXB   = 0b1011001011'000'000,
		REV   = 0b1011101000'000'000,
		REV16 = 0b1011101001'000'000,
		REVSH = 0b1011101011'000'000,
	};

	static inline uint16_t reg2(Reg2Op op, Reg dn, Reg m) {
		assert(dn < 8 && m < 8);
		return (uint16_t)op | (m << 3) | dn;
	}

	enum class Reg3Op: uint16_t
	{
		// 	        opcode  m   n  d/t
		ADDREG = 0b0001100'000'000'000,
		SUBREG = 0b0001101'000'000'000,
		ADDIMM = 0b0001110'000'000'000,
		SUBIMM = 0b0001111'000'000'000,
		STR    = 0b0101000'000'000'000,
		STRH   = 0b0101001'000'000'000,
		STRB   = 0b0101010'000'000'000,
		LDRSB  = 0b0101011'000'000'000,
		LDR    = 0b0101110'000'000'000,
		LDRH   = 0b0101101'000'000'000,
		LDRB   = 0b0101110'000'000'000,
		LDRSH  = 0b0101111'000'000'000,
	};

	static inline uint16_t reg3(Reg3Op op, Reg dt, Reg n, Reg m)
	{
		assert(dt < 8 && m < 8 && n < 8);
		return (uint16_t)op | (m << 6) | (n << 3) | dt;
	}

	enum class Imm5Op: uint16_t
	{
		//      opcode  imm5 m/n d/t
		LSL  = 0b00000'00000'000'000,
		LSR  = 0b00001'00000'000'000,
		ASR  = 0b00010'00000'000'000,
		STR  = 0b01100'00000'000'000,
		LDR  = 0b01101'00000'000'000,
		STRB = 0b01110'00000'000'000,
		LDRB = 0b01111'00000'000'000,
		STRH = 0b10000'00000'000'000,
		LDRH = 0b10001'00000'000'000,
	};

	static inline uint16_t imm5(Imm5Op op, Reg dt, Reg mn, uint16_t imm5)
	{
		assert(dt < 8 && mn < 8 && imm5 < 32);
		return (uint16_t)op | (imm5 << 6) | (mn << 3) | dt;
	}

	enum class Imm7Op: uint16_t
	{
		//            opcode   imm7
		INCRSP = 0b101100000'0000000,
		DECRSP = 0b101100001'0000000,
	};

	static inline uint16_t imm7(Imm7Op op, uint16_t imm7)
	{
		assert(imm7 < 128);
		return (uint16_t)op | imm7;
	}

	enum class Imm8Op: uint16_t
	{
		//       opcode d/n   imm8
		MOV   = 0b00100'000'00000000,
		CMP   = 0b00101'000'00000000,
		ADD   = 0b00110'000'00000000,
		SUB   = 0b00111'000'00000000,
		STRSP = 0b10010'000'00000000,
		LDRSP = 0b10011'000'00000000,
		LDR   = 0b01001'000'00000000,
		ADR   = 0b10100'000'00000000,
		ADDSP = 0b10101'000'00000000,
	};

	static inline uint16_t imm8(Imm8Op op, Reg r, uint16_t imm8)
	{
		assert(r < 8 && imm8 < 256);
		return (uint16_t) op | (r << 8) | imm8;
	}

	enum class NoArgOp: uint16_t
	{
		CPSIE = 0b1011'0110'0110'0010,
		CPSID = 0b1011'0110'0111'0010,
		NOP   = 0b1011'1111'0000'0000,
		YIELD = 0b1011'1111'0001'0000,
		WFE   = 0b1011'1111'0010'0000,
		WFI   = 0b1011'1111'0011'0000,
		SEV   = 0b1011'1111'0100'0000,
	};

	static inline uint16_t noArg(NoArgOp op) {
		return (uint16_t)op;
	}

	enum class BranchOp
	{
		EQ  = 0b1101'0000'00000000,	// Equal 					  Z == 1
		NE  = 0b1101'0001'00000000,	// Not equal 				  Z == 0
		HS  = 0b1101'0010'00000000,	// Unsigned higher or same 	  C == 1
		LO  = 0b1101'0011'00000000,	// Unsigned lower 			  C == 0
		MI  = 0b1101'0100'00000000,	// Minus, negative 			  N == 1
		PL  = 0b1101'0101'00000000,	// Plus, positive 			  N == 0
		VS  = 0b1101'0110'00000000,	// Overflow 				  V == 1
		VC  = 0b1101'0111'00000000,	// No overflow 				  V == 0
		HI  = 0b1101'1000'00000000,	// Unsigned higher 			  C == 1 && Z == 0
		LS  = 0b1101'1001'00000000,	// Unsigned lower or same 	  C == 0 && Z == 1
		GE  = 0b1101'1010'00000000,	// Signed greater or equal    N == V
		LT  = 0b1101'1010'00000000,	// Signed less than           N != V
		GT  = 0b1101'1010'00000000,	// Signed greater than        Z == 0 && N == V
		LE  = 0b1101'1101'00000000,	// Signed less than or equal  Z == 1 || N != V
		AL  = 0b1101'1110'00000000,	// Always
		UDF = 0b1101'1110'00000000,	// Permanently undefined
		SVC = 0b1101'1111'00000000,	// Supervisor call
	};

	static inline uint16_t branchSvc(BranchOp op, uint16_t imm8)
	{
		assert(imm8 < 256);
		return (uint16_t)op << 8 | imm8;
	}

	enum class HiRegOp: uint16_t
	{
		ADD = 0b01000100,
		CMP = 0b01000101,
		MOV = 0b01000110,
		JMP = 0b01000111,
	};

	static inline uint16_t hiReg(HiRegOp op, Reg dn, Reg m)
	{
		assert(dn < 16 && m < 16);
		return (uint16_t)op  | (((uint16_t)dn >> 3) << 7) | m << 3 | (dn & 0b0111);
	}


	//      TTTTT  BBBB   DDDD
	//        T    B   B  D   D
	//        T    BBBB   D   D
	//        T    B   B  D   D
	//        T    BBBB   DDDD
	//
    //     ˇˇˇˇˇˇˇˇˇˇˇˇˇˇˇˇˇˇˇˇˇˇ


	struct LowRegisterList
	{
		uint8_t flags;

		inline void addRegister(Reg r) {
			flags |= 1 << r;
		}
	};

	static inline uint16_t push(LowRegisterList regFlags, bool includeLr) {
		return 0b1011'0'10'0'00000000 | ((uint16_t)includeLr << 8) | regFlags;
	}

	static inline uint16_t pop(LowRegisterList regFlags, bool includePc) {
		return 0b1011'1'10'0'00000000 | ((uint16_t)includePc << 8) | regFlags;
	}

	static inline uint16_t lsMia(bool load, Reg n, LowRegisterList regFlags)
	{
		assert(n < 16);
		return 0b11000'00000000000 | ((uint16_t)load << 8) | regFlags;
	}

public:
	static inline uint16_t ands(Reg dn, Reg m) { return reg2(Reg2Op::AND, dn, m); }
	static inline uint16_t eors(Reg dn, Reg m) { return reg2(Reg2Op::EOR, dn, m); }
	static inline uint16_t lsls(Reg dn, Reg m) { return reg2(Reg2Op::LSL, dn, m); }
	static inline uint16_t lsrs(Reg dn, Reg m) { return reg2(Reg2Op::LSR, dn, m); }
	static inline uint16_t asrs(Reg dn, Reg m) { return reg2(Reg2Op::ASR, dn, m); }
	static inline uint16_t adcs(Reg dn, Reg m) { return reg2(Reg2Op::ADC, dn, m); }
	static inline uint16_t sbcs(Reg dn, Reg m) { return reg2(Reg2Op::SBC, dn, m); }
	static inline uint16_t rors(Reg dn, Reg m) { return reg2(Reg2Op::ROR, dn, m); }
	static inline uint16_t tst(Reg n, Reg m)   { return reg2(Reg2Op::TST, n, m); }
	static inline uint16_t rsb(Reg d, Reg m)   { return reg2(Reg2Op::RSB, d, m); }
	static inline uint16_t cmp(Reg n, Reg m)   { return reg2(Reg2Op::CMP, n, m); }
	static inline uint16_t cmn(Reg n, Reg m)   { return reg2(Reg2Op::CMN, n, m); }
	static inline uint16_t orrs(Reg dn, Reg m) { return reg2(Reg2Op::ORR, dn, m); }
	static inline uint16_t muls(Reg dn, Reg m) { return reg2(Reg2Op::MUL, dn, m); }
	static inline uint16_t bics(Reg dn, Reg m) { return reg2(Reg2Op::BIC, dn, m); }
	static inline uint16_t mvns(Reg dn, Reg m) { return reg2(Reg2Op::MVN, dn, m); }
	static inline uint16_t sxth(Reg d, Reg m)  { return reg2(Reg2Op::SXH, d, m); }
	static inline uint16_t sxtb(Reg d, Reg m)  { return reg2(Reg2Op::SXB, d, m); }
	static inline uint16_t uxth(Reg d, Reg m)  { return reg2(Reg2Op::UXH, d, m); }
	static inline uint16_t uxtb(Reg d, Reg m)  { return reg2(Reg2Op::UXB, d, m); }
	static inline uint16_t rev(Reg d, Reg m)   { return reg2(Reg2Op::REV, d, m); }
	static inline uint16_t rev16(Reg d, Reg m) { return reg2(Reg2Op::REV16, d, m); }
	static inline uint16_t revsh(Reg d, Reg m) { return reg2(Reg2Op::REVSH, d, m); }

	static inline uint16_t addsReg(Reg d, Reg n, Reg m)          { return reg3(Reg3Op::ADDREG, d, n, m); }
	static inline uint16_t addsImm3(Reg d, Reg n, uint16_t imm)  { return reg3(Reg3Op::ADDIMM, d, n, imm); }
	static inline uint16_t subsReg(Reg d, Reg n, Reg m)          { return reg3(Reg3Op::SUBREG, d, n, m); }
	static inline uint16_t subsImm3(Reg d, Reg n, uint16_t imm)  { return reg3(Reg3Op::SUBIMM, d, n, imm); }
	static inline uint16_t str(Reg t, Reg n, Reg m)              { return reg3(Reg3Op::STR,    t, n, m); }
	static inline uint16_t strh(Reg t, Reg n, Reg m)             { return reg3(Reg3Op::STRH,   t, n, m); }
	static inline uint16_t strb(Reg t, Reg n, Reg m)             { return reg3(Reg3Op::STRB,   t, n, m); }
	static inline uint16_t ldrsb(Reg t, Reg n, Reg m)            { return reg3(Reg3Op::LDRSB,  t, n, m); }
	static inline uint16_t ldr(Reg t, Reg n, Reg m)              { return reg3(Reg3Op::LDR,    t, n, m); }
	static inline uint16_t ldrh(Reg t, Reg n, Reg m)             { return reg3(Reg3Op::LDRH,   t, n, m); }
	static inline uint16_t ldrb(Reg t, Reg n, Reg m)             { return reg3(Reg3Op::LDRB,   t, n, m); }
	static inline uint16_t ldrsh(Reg t, Reg n, Reg m)            { return reg3(Reg3Op::LDRSH,  t, n, m); }

	static inline uint16_t lslsImm5(Reg d, Reg m, uint16_t imm) { return imm5(Imm5Op::LSL, d, m, imm); }
	static inline uint16_t lsrsImm5(Reg d, Reg m, uint16_t imm) { return imm5(Imm5Op::LSR, d, m, imm); }
	static inline uint16_t asrsImm5(Reg d, Reg m, uint16_t imm) { return imm5(Imm5Op::ASR, d, m, imm); }
	static inline uint16_t strImm5(Reg t, Reg n, uint16_t imm)  { return imm5(Imm5Op::STR, t, n, imm); }
	static inline uint16_t ldrImm5(Reg t, Reg n, uint16_t imm)  { return imm5(Imm5Op::LDR, t, n, imm); }
	static inline uint16_t strbImm5(Reg t, Reg n, uint16_t imm) { return imm5(Imm5Op::STRB, t, n, imm); }
	static inline uint16_t ldrbImm5(Reg t, Reg n, uint16_t imm) { return imm5(Imm5Op::LDRB, t, n, imm); }
	static inline uint16_t strhImm5(Reg t, Reg n, uint16_t imm) { return imm5(Imm5Op::STRH, t, n, imm); }
	static inline uint16_t ldrhImm5(Reg t, Reg n, uint16_t imm) { return imm5(Imm5Op::LDRH, t, n, imm); }

	static inline uint16_t incrSp(uint16_t imm) { return imm7(Imm7Op::INCRSP, imm); }
	static inline uint16_t decrSp(uint16_t imm) { return imm7(Imm7Op::DECRSP, imm); }

	static inline uint16_t movsImm8(Reg d, uint16_t imm)  { return imm8(Imm8Op::MOV, d, imm); }
	static inline uint16_t cmpImm8(Reg n, uint16_t imm)   { return imm8(Imm8Op::CMP, n, imm); }
	static inline uint16_t addsImm8(Reg dn, uint16_t imm) { return imm8(Imm8Op::ADD, dn, imm); }
	static inline uint16_t subsImm8(Reg dn, uint16_t imm) { return imm8(Imm8Op::SUB, dn, imm); }
	static inline uint16_t strSp(Reg t, uint16_t imm)     { return imm8(Imm8Op::STRSP, t, imm); }
	static inline uint16_t ldrSp(Reg t, uint16_t imm)     { return imm8(Imm8Op::LDRSP, t, imm); }
	static inline uint16_t ldrPc(Reg t, uint16_t imm)     { return imm8(Imm8Op::LDR, t, imm); }
	static inline uint16_t adr(Reg d, uint16_t imm)       { return imm8(Imm8Op::LDR, d, imm); }
	static inline uint16_t addSp(Reg d, uint16_t imm)     { return imm8(Imm8Op::LDR, d, imm); }

	static inline uint16_t bkpt(uint8_t imm8) {
		return 0b10111110'00000000 | imm8;
	}

	static inline uint16_t cpsie() { return noArg(NoArgOp::CPSIE); }
	static inline uint16_t cpsid() { return noArg(NoArgOp::CPSID); }
	static inline uint16_t nop()   { return noArg(NoArgOp::NOP); }
	static inline uint16_t yield() { return noArg(NoArgOp::YIELD); }
	static inline uint16_t wfe()   { return noArg(NoArgOp::WFE); }
	static inline uint16_t wfi()   { return noArg(NoArgOp::WFI); }
	static inline uint16_t sev()   { return noArg(NoArgOp::SEV); }

	static inline uint16_t beq(uint16_t imm) { return branchSvc(BranchOp::EQ, imm); }
	static inline uint16_t bne(uint16_t imm) { return branchSvc(BranchOp::NE, imm); }
	static inline uint16_t bhs(uint16_t imm) { return branchSvc(BranchOp::HS, imm); }
	static inline uint16_t blo(uint16_t imm) { return branchSvc(BranchOp::LO, imm); }
	static inline uint16_t bmi(uint16_t imm) { return branchSvc(BranchOp::MI, imm); }
	static inline uint16_t bpl(uint16_t imm) { return branchSvc(BranchOp::PL, imm); }
	static inline uint16_t bvs(uint16_t imm) { return branchSvc(BranchOp::VS, imm); }
	static inline uint16_t bvc(uint16_t imm) { return branchSvc(BranchOp::VC, imm); }
	static inline uint16_t bhi(uint16_t imm) { return branchSvc(BranchOp::HI, imm); }
	static inline uint16_t bls(uint16_t imm) { return branchSvc(BranchOp::LS, imm); }
	static inline uint16_t bge(uint16_t imm) { return branchSvc(BranchOp::GE, imm); }
	static inline uint16_t blt(uint16_t imm) { return branchSvc(BranchOp::LT, imm); }
	static inline uint16_t bgt(uint16_t imm) { return branchSvc(BranchOp::GT, imm); }
	static inline uint16_t ble(uint16_t imm) { return branchSvc(BranchOp::LE, imm); }
	static inline uint16_t bal(uint16_t imm) { return branchSvc(BranchOp::AL, imm); }
	static inline uint16_t udf(uint16_t imm) { return branchSvc(BranchOp::UDF, imm); }
	static inline uint16_t svc(uint16_t imm) { return branchSvc(BranchOp::SVC, imm); }

	static inline uint16_t addHi(Reg dn, Reg m) { return hiReg(HiRegOp::ADD, dn, m); }
	static inline uint16_t movHi(Reg dn, Reg m) { return hiReg(HiRegOp::MOV, dn, m); }
	static inline uint16_t blx(Reg m)           { return hiReg(HiRegOp::JMP, 0b1000, m); }
	static inline uint16_t bx(Reg m)            { return hiReg(HiRegOp::JMP, 0b0000, m); }
	static inline uint16_t cmpHi(Reg n, Reg m)
	{
		assert((n & 0b1000) || (m & 0b1000));
		return hiReg(HiRegOp::CMP, n, m);
	}

	static inline uint16_t branchImm11(uint16_t imm11)
	{
		assert(imm11 < 4096);
		return 0b11100'00000000000 | imm11;
	}
};

#endif /* JIT_ARMV6_H_ */
