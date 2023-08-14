#ifndef INSTRUCTION_H_
#define INSTRUCTION_H_

#include <cstdint>
#include <cstddef>

namespace prog {

#define FMT0(n) static constexpr inline Instruction n(Reg x, uint32_t imm)          { return {Operation:: n, x, imm}; }
#define FMT1(n) static constexpr inline Instruction n(Reg x, Reg y)                 { return {Operation:: n, x, y}; }
#define FMT2(n) static constexpr inline Instruction n(Reg x, Reg y, uint32_t imm)   { return {Operation:: n, x, y, imm}; }
#define FMT3(n) static constexpr inline Instruction n(Reg x, Reg y, Reg z)          { return {Operation:: n, x, y, z}; }
#define FMT4(n) static constexpr inline Instruction n(uint32_t imm)                 { return {Operation:: n, imm}; }
#define FMT5(n) static constexpr inline Instruction n(uint32_t immM, uint32_t immN) { return {Operation:: n, immM, immN}; }

#define OPERATION_LIST() \
	X(lit, FMT0) \
	X(movr, FMT1) \
	X(mov, FMT1) \
	X(neg, FMT1) \
	X(i2f, FMT1) \
	X(f2i, FMT1) \
	X(x1i, FMT1) \
	X(x1u, FMT1) \
	X(x2i, FMT1) \
	X(x2u, FMT1) \
	X(getr, FMT2) \
	X(putr, FMT2) \
	X(gets, FMT2) \
	X(puts, FMT2) \
	X(jEq, FMT2) \
	X(jNe, FMT2) \
	X(jLtI, FMT2) \
	X(jGtI, FMT2) \
	X(jLeI, FMT2) \
	X(jGeI, FMT2) \
	X(jLtU, FMT2) \
	X(jGtU, FMT2) \
	X(jLeU, FMT2) \
	X(jGeU, FMT2) \
	X(jLtF, FMT2) \
	X(jGtF, FMT2) \
	X(jLeF, FMT2) \
	X(jGeF, FMT2) \
	X(addI, FMT3) \
	X(mulI, FMT3) \
	X(subI, FMT3) \
	X(divI, FMT3) \
	X(mod, FMT3) \
	X(shlI, FMT3) \
	X(shrI, FMT3) \
	X(shrU, FMT3) \
	X(andI, FMT3) \
	X(orI, FMT3) \
	X(xorI, FMT3) \
	X(addF, FMT3) \
	X(mulF, FMT3) \
	X(subF, FMT3) \
	X(divF, FMT3) \
	X(jump, FMT4) \
	X(raise, FMT4) \
	X(drop, FMT4) \
	X(call, FMT5) \
	X(ret, FMT5)

struct Instruction
{
	struct Reg
	{
		enum class Kind {
			Tos, Local, Global
		};

		Kind kind;
		uint16_t index;

		inline constexpr Reg(): kind(Kind::Tos), index(0) {}
		inline constexpr Reg(uint16_t index): kind(Kind::Local), index(index) {}
	};

	enum class Operation
	{
#define X(name, fmt) name,
		OPERATION_LIST()
#undef X
	};

	Operation op = Operation::lit;
	Reg x, y, z;
	uint32_t imm = 0, imm2 = 0;

	inline constexpr Instruction() = default;
	inline constexpr Instruction(Operation op, Reg x, uint32_t imm): op(op), x(x), imm(imm) {}
	inline constexpr Instruction(Operation op, Reg x, Reg y): op(op), x(x), y(y) {}
	inline constexpr Instruction(Operation op, Reg x, Reg y, uint32_t imm): op(op), x(x), y(y), imm(imm) {}
	inline constexpr Instruction(Operation op, Reg x, Reg y, Reg z): op(op), x(x), y(y), z(z) {}
	inline constexpr Instruction(Operation op, uint32_t imm): op(op), imm(imm) {}
	inline constexpr Instruction(Operation op, uint32_t imm, uint32_t imm2): op(op), imm(imm), imm2(imm2) {}

#define X(name, fmt) fmt(name)
		OPERATION_LIST()
#undef X
};

} //namespace prog

#endif /* INSTRUCTION_H_ */
