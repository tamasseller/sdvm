#ifndef DISASSEMBLER_H_
#define DISASSEMBLER_H_

#include "jit/armv6.h"

#include <vector>
#include <string>
#include <sstream>
#include <cassert>

class Disassembler: ArmV6
{
	static inline std::string regName(uint16_t r)
	{
		static constexpr const char* names[] =
		{
			"r0", "r1", "r2",  "r3",  "r4",  "r5", "r6", "r7",
			"r8", "r9", "r10", "r11", "r12", "sp", "lr", "pc",
		};

		return names[r & 15];
	}

	static inline std::string loRegName(uint16_t r) {
		return regName(r & 7);
	}

	static inline std::string displayReg2(const std::string& mn, uint16_t isn) {
		return mn + " " + loRegName(isn) + ", " + loRegName(isn >> 3);
	}

	static inline std::string displayReg3(const std::string& mn, uint16_t isn) {
		return mn + " " + loRegName(isn) + ", " + loRegName(isn >> 3) + ", " + loRegName(isn >> 6);
	}

	static inline std::string displayReg3i(const std::string& mn, uint16_t isn) {
		return mn + " " + loRegName(isn) + ", " + loRegName(isn >> 3) + ", #" + std::to_string((isn >> 6) & 7);
	}

	static inline std::string displayReg3ls(const std::string& mn, uint16_t isn) {
		return mn + " " + loRegName(isn) + ", [" + loRegName(isn >> 3) + ", " + loRegName(isn >> 6) + "]";
	}

	static inline std::string displayImm5(const std::string& mn, uint16_t isn) {
		return mn + " " + loRegName(isn) + ", " + loRegName(isn >> 3) + ", #" + std::to_string((isn >> 6) & 31);
	}

	static inline std::string displayImm5ls(const std::string& mn, uint16_t isn, uint16_t shift) {
		return mn + " " + loRegName(isn) + ", [" + loRegName(isn >> 3) + ", #" + std::to_string(((isn >> 6) & 31) << shift) + "]";
	}

	static inline std::string displayImm7(const std::string& mn, uint16_t isn) {
		return mn + " sp, #" + std::to_string((isn & 127) << 2);
	}

	static inline std::string displayImm8(const std::string& mn, uint16_t isn) {
		return mn + " " + loRegName(isn >> 8) + ", #" + std::to_string(isn & 255);
	}

	static inline std::string displayImm8s(const std::string& mn, const std::string& base, uint16_t isn, bool boxed) {
		return mn + " " + loRegName(isn >> 8) + ", "+ (boxed ? "[" : "") + base + ", #" + std::to_string((isn & 255) << 2) + (boxed ? "]" : "");
	}

	static inline std::string displayCond(const std::string& mn, uint16_t isn) {
		return mn + " " + std::to_string(((int)(int8_t)(isn & 255)) << 1);
	}

	static inline std::string displayUdfSvcBkpt(const std::string& mn, uint16_t isn) {
		return mn + " #" + std::to_string(isn & 255);
	}

	static inline std::string displayHireg(const std::string& mn, uint16_t dn, uint16_t m) {
		return mn + " " + regName(dn) + ", " + regName(m);
	}

	static inline std::string formatRegList(uint16_t isn, const char *extra = nullptr)
	{
		std::stringstream ss;
		ss << "{";

		const char* sep = "";
		for(auto i = 0u; i < 8; i++)
		{
			if(isn & (1 << i))
			{
				ss << sep << loRegName(i);
				sep = ", ";
			}
		}

		if(extra)
		{
			ss << sep << extra;
		}

		ss << "}";
		return ss.str();
	}

public:
	static std::vector<std::string> disassemble(uint16_t *from, uint16_t *to)
	{
		std::vector<std::string> ret;

		while(from != to)
		{
			auto isn = *from++;

			switch(isn >> 6)
			{
			case (uint16_t)Reg2Op::AND >> 6:
				ret.push_back(displayReg2("ands", isn));
				continue;
			case (uint16_t)Reg2Op::EOR >> 6:
				ret.push_back(displayReg2("eors", isn));
				continue;
			case (uint16_t)Reg2Op::LSL >> 6:
				ret.push_back(displayReg2("lsls", isn));
				continue;
			case (uint16_t)Reg2Op::LSR >> 6:
				ret.push_back(displayReg2("lsrs", isn));
				continue;
			case (uint16_t)Reg2Op::ASR >> 6:
				ret.push_back(displayReg2("asrs", isn));
				continue;
			case (uint16_t)Reg2Op::ADC >> 6:
				ret.push_back(displayReg2("adcs", isn));
				continue;
			case (uint16_t)Reg2Op::SBC >> 6:
				ret.push_back(displayReg2("sbcs", isn));
				continue;
			case (uint16_t)Reg2Op::ROR >> 6:
				ret.push_back(displayReg2("rors", isn));
				continue;
			case (uint16_t)Reg2Op::TST >> 6:
				ret.push_back(displayReg2("tst", isn));
				continue;
			case (uint16_t)Reg2Op::RSB >> 6:
				ret.push_back(displayReg2("negs", isn));
				continue;
			case (uint16_t)Reg2Op::CMP >> 6:
				ret.push_back(displayReg2("cmp", isn));
				continue;
			case (uint16_t)Reg2Op::CMN >> 6:
				ret.push_back(displayReg2("cmn", isn));
				continue;
			case (uint16_t)Reg2Op::ORR >> 6:
				ret.push_back(displayReg2("orrs", isn));
				continue;
			case (uint16_t)Reg2Op::MUL >> 6:
				ret.push_back(displayReg2("muls", isn));
				continue;
			case (uint16_t)Reg2Op::BIC >> 6:
				ret.push_back(displayReg2("bics", isn));
				continue;
			case (uint16_t)Reg2Op::MVN >> 6:
				ret.push_back(displayReg2("mvns", isn));
				continue;
			case (uint16_t)Reg2Op::SXH >> 6:
				ret.push_back(displayReg2("sxth", isn));
				continue;
			case (uint16_t)Reg2Op::SXB >> 6:
				ret.push_back(displayReg2("sxtb", isn));
				continue;
			case (uint16_t)Reg2Op::UXH >> 6:
				ret.push_back(displayReg2("uxth", isn));
				continue;
			case (uint16_t)Reg2Op::UXB >> 6:
				ret.push_back(displayReg2("uxtb", isn));
				continue;
			case (uint16_t)Reg2Op::REV >> 6:
				ret.push_back(displayReg2("rev", isn));
				continue;
			case (uint16_t)Reg2Op::REV16 >> 6:
				ret.push_back(displayReg2("rev16", isn));
				continue;
			case (uint16_t)Reg2Op::REVSH >> 6:
				ret.push_back(displayReg2("revsh", isn));
				continue;
			}

			switch(isn >> 9)
			{
			case (uint16_t)Reg3Op::ADDREG >> 9:
				ret.push_back(displayReg3("adds", isn));
				continue;
			case (uint16_t)Reg3Op::SUBREG >> 9:
				ret.push_back(displayReg3("subs", isn));
				continue;
			case (uint16_t)Reg3Op::ADDIMM >> 9:
				ret.push_back(displayReg3i("adds", isn));
				continue;
			case (uint16_t)Reg3Op::SUBIMM >> 9:
				ret.push_back(displayReg3i("subs", isn));
				continue;
			case (uint16_t)Reg3Op::STR >> 9:
				ret.push_back(displayReg3ls("str", isn));
				continue;
			case (uint16_t)Reg3Op::STRH >> 9:
				ret.push_back(displayReg3ls("strh", isn));
				continue;
			case (uint16_t)Reg3Op::STRB >> 9:
				ret.push_back(displayReg3ls("strb", isn));
				continue;
			case (uint16_t)Reg3Op::LDRSB >> 9:
				ret.push_back(displayReg3ls("ldrsb", isn));
				continue;
			case (uint16_t)Reg3Op::LDR >> 9:
				ret.push_back(displayReg3ls("ldr", isn));
				continue;
			case (uint16_t)Reg3Op::LDRH >> 9:
				ret.push_back(displayReg3ls("ldrh", isn));
				continue;
			case (uint16_t)Reg3Op::LDRB >> 9:
				ret.push_back(displayReg3ls("ldrb", isn));
				continue;
			case (uint16_t)Reg3Op::LDRSH >> 9:
				ret.push_back(displayReg3ls("ldrsh", isn));
				continue;
			}

			switch(isn >> 11)
			{
			case (uint16_t)Imm5Op::LSL >> 11:
				ret.push_back(displayImm5("lsls", isn));
				continue;
			case (uint16_t)Imm5Op::LSR >> 11:
				ret.push_back(displayImm5("lsrs", isn));
				continue;
			case (uint16_t)Imm5Op::ASR >> 11:
				ret.push_back(displayImm5("asrs", isn));
				continue;
			case (uint16_t)Imm5Op::STR >> 11:
				ret.push_back(displayImm5ls("str", isn, 2));
				continue;
			case (uint16_t)Imm5Op::LDR >> 11:
				ret.push_back(displayImm5ls("ldr", isn, 2));
				continue;
			case (uint16_t)Imm5Op::STRB >> 11:
				ret.push_back(displayImm5ls("strb", isn, 0));
				continue;
			case (uint16_t)Imm5Op::LDRB >> 11:
				ret.push_back(displayImm5ls("ldrb", isn, 0));
				continue;
			case (uint16_t)Imm5Op::STRH >> 11:
				ret.push_back(displayImm5ls("strh", isn, 1));
				continue;
			case (uint16_t)Imm5Op::LDRH >> 11:
				ret.push_back(displayImm5ls("ldrh", isn, 1));
				continue;
			}

			switch(isn >> 7)
			{
			case (uint16_t)Imm7Op::INCRSP >> 7:
				ret.push_back(displayImm7("add", isn));
				continue;
			case (uint16_t)Imm7Op::DECRSP >> 7:
				ret.push_back(displayImm7("sub", isn));
				continue;
			}

			switch(isn >> 11)
			{
			case (uint16_t)Imm8Op::MOV >> 11:
				ret.push_back(displayImm8("movs", isn));
				continue;
			case (uint16_t)Imm8Op::CMP >> 11:
				ret.push_back(displayImm8("cmp", isn));
				continue;
			case (uint16_t)Imm8Op::ADD >> 11:
				ret.push_back(displayImm8("adds", isn));
				continue;
			case (uint16_t)Imm8Op::SUB >> 11:
				ret.push_back(displayImm8("subs", isn));
				continue;
			case (uint16_t)Imm8Op::STRSP >> 11:
				ret.push_back(displayImm8s("str", "sp", isn, true));
				continue;
			case (uint16_t)Imm8Op::LDRSP >> 11:
				ret.push_back(displayImm8s("ldr", "sp", isn, true));
				continue;
			case (uint16_t)Imm8Op::LDR >> 11:
				ret.push_back(displayImm8s("ldr", "pc", isn, true));
				continue;
			case (uint16_t)Imm8Op::ADR >> 11:
				ret.push_back(displayImm8s("add", "pc", isn, false));
				continue;
			case (uint16_t)Imm8Op::ADDSP >> 11:
				ret.push_back(displayImm8s("add", "sp", isn, false));
				continue;
			}

			switch(isn)
			{
			case (uint16_t)NoArgOp::CPSIE:
				ret.push_back("cpsie i");
				continue;
			case (uint16_t)NoArgOp::CPSID:
				ret.push_back("cpsid i");
				continue;
			case (uint16_t)NoArgOp::NOP:
				ret.push_back("nop");
				continue;
			case (uint16_t)NoArgOp::YIELD:
				ret.push_back("yield");
				continue;
			case (uint16_t)NoArgOp::WFE:
				ret.push_back("wfe");
				continue;
			case (uint16_t)NoArgOp::WFI:
				ret.push_back("wfi");
				continue;
			case (uint16_t)NoArgOp::SEV:
				ret.push_back("sev");
				continue;
			}

			switch(isn >> 8)
			{
			case (uint16_t)BranchOp::EQ >> 8:
				ret.push_back(displayCond("beq", isn));
				continue;
			case (uint16_t)BranchOp::NE >> 8:
				ret.push_back(displayCond("bne", isn));
				continue;
			case (uint16_t)BranchOp::HS >> 8:
				ret.push_back(displayCond("bhs", isn));
				continue;
			case (uint16_t)BranchOp::LO >> 8:
				ret.push_back(displayCond("blo", isn));
				continue;
			case (uint16_t)BranchOp::MI >> 8:
				ret.push_back(displayCond("bmi", isn));
				continue;
			case (uint16_t)BranchOp::PL >> 8:
				ret.push_back(displayCond("bpl", isn));
				continue;
			case (uint16_t)BranchOp::VS >> 8:
				ret.push_back(displayCond("bvs", isn));
				continue;
			case (uint16_t)BranchOp::VC >> 8:
				ret.push_back(displayCond("bvc", isn));
				continue;
			case (uint16_t)BranchOp::HI >> 8:
				ret.push_back(displayCond("bhi", isn));
				continue;
			case (uint16_t)BranchOp::LS >> 8:
				ret.push_back(displayCond("bls", isn));
				continue;
			case (uint16_t)BranchOp::GE >> 8:
				ret.push_back(displayCond("bge", isn));
				continue;
			case (uint16_t)BranchOp::LT >> 8:
				ret.push_back(displayCond("blt", isn));
				continue;
			case (uint16_t)BranchOp::GT >> 8:
				ret.push_back(displayCond("bgt", isn));
				continue;
			case (uint16_t)BranchOp::LE >> 8:
				ret.push_back(displayCond("ble", isn));
				continue;
			case (uint16_t)BranchOp::UDF >> 8:
				ret.push_back(displayUdfSvcBkpt("udf", isn));
				continue;
			case (uint16_t)BranchOp::SVC >> 8:
				ret.push_back(displayUdfSvcBkpt("svc", isn));
				continue;
			}

			if((isn & 0b1111'0'11'0'00000000) == 0b1011'0'10'0'00000000)
			{
				if(isn & (1 << 11))
				{
					ret.push_back("pop " + formatRegList(isn, (isn & (1 << 8)) ? "pc" : nullptr));
					continue;
				}
				else
				{
					ret.push_back("push " + formatRegList(isn, (isn & (1 << 8)) ? "lr" : nullptr));
					continue;
				}
			}

			if((isn & 0b1111'0'000'00000000) == 0b1100'0'000'00000000)
			{
				if(isn & (1 << 11))
				{
					const auto n = (isn >> 8) & 7;
					const auto wb = (isn & (1 << n)) == 0;
					ret.push_back("ldmia " + loRegName(n) + (wb ? "!, " : ", ") + formatRegList(isn));
					continue;
				}
				else
				{
					ret.push_back("stmia " + loRegName(isn >> 8) + "!, " + formatRegList(isn));
					continue;
				}
			}

			if((isn & 0b1111'0'000'00000000) == 0b1100'0'000'00000000)
			{
				if(isn & (1 << 11))
				{
					const auto n = (isn >> 8) & 7;
					const auto wb = (isn & (1 << n)) == 0;
					ret.push_back("ldmia " + loRegName(n) + (wb ? "!, " : ", ") + formatRegList(isn));
					continue;
				}
				else
				{
					ret.push_back("stmia " + loRegName(isn >> 8) + "!, " + formatRegList(isn));
					continue;
				}
			}

			if((isn & 0b11111111'00000000) == 0b10111110'00000000)
			{
				ret.push_back(displayUdfSvcBkpt("bkpt", isn));
				continue;
			}

			if((isn & 0b11111'00000000000) == 0b11100'00000000000)
			{
				int16_t off = isn & 0x7ff;
				off |= (isn & 0x0400) ? 0xf800 : 0;
				ret.push_back("b " + std::to_string(off << 1));
				continue;
			}

			const auto dn = (isn & 7) | ((isn >> 4) & 0b1000);
			const auto m = (isn >> 3) & 15;

			switch(isn >> 8)
			{
				case (uint16_t)HiRegOp::ADD >> 8:
					ret.push_back(displayHireg("add", dn, m));
					continue;
				case (uint16_t)HiRegOp::CMP >> 8:
					ret.push_back(displayHireg("cmp", dn, m));
					continue;
				case (uint16_t)HiRegOp::MOV >> 8:
					ret.push_back(displayHireg("mov", dn, m));
					continue;
				case (uint16_t)HiRegOp::JMP >> 8:
					if(dn == 0)
					{
						ret.push_back("bx " + regName(m));
					}
					else
					{
						assert(dn == 0b1000);
						ret.push_back("blx " + regName(m));
						// TODO if m == 9 next is data
					}
					continue;
			}
		}

		return ret;
	}
};

#endif /* DISASSEMBLER_H_ */
