#ifndef INSTRUCTION_H_
#define INSTRUCTION_H_

#include "concept/Binary.h"

#include <cstddef>

namespace prog {

#define FMT0(n) static constexpr inline Instruction n(Reg x, uint32_t imm)          { return {Operation:: n, x, imm}; }
#define FMT1(n) static constexpr inline Instruction n(Reg x, Reg y)                 { return {Operation:: n, x, y}; }
#define FMT2(n) static constexpr inline Instruction n(Reg x, Reg y, uint32_t imm)   { return {Operation:: n, x, y, imm}; }
#define FMT3(n) static constexpr inline Instruction n(Reg x, Reg y, Reg z)          { return {Operation:: n, x, y, z}; }
#define FMT4(n) static constexpr inline Instruction n(uint32_t imm)                 { return {Operation:: n, imm}; }
#define FMT5(n) static constexpr inline Instruction n(uint32_t immM, uint32_t immN) { return {Operation:: n, immM, immN}; }

#define OPERATION_LIST(CONSUMER) \
	CONSUMER(lit, FMT0) \
	CONSUMER(make, FMT0) \
	CONSUMER(jNul, FMT0) \
	CONSUMER(jNnl, FMT0) \
	CONSUMER(movr, FMT1) \
	CONSUMER(mov, FMT1) \
	CONSUMER(neg, FMT1) \
	CONSUMER(i2f, FMT1) \
	CONSUMER(f2i, FMT1) \
	/*CONSUMER(x1i, FMT1)*/ \
	/*CONSUMER(x1u, FMT1)*/ \
	/*CONSUMER(x2i, FMT1)*/ \
	/*CONSUMER(x2u, FMT1)*/ \
	CONSUMER(getr, FMT2) \
	CONSUMER(putr, FMT2) \
	CONSUMER(gets, FMT2) \
	CONSUMER(puts, FMT2) \
	CONSUMER(jEq, FMT2) \
	CONSUMER(jNe, FMT2) \
	CONSUMER(jLtI, FMT2) \
	CONSUMER(jGtI, FMT2) \
	CONSUMER(jLeI, FMT2) \
	CONSUMER(jGeI, FMT2) \
	CONSUMER(jLtU, FMT2) \
	CONSUMER(jGtU, FMT2) \
	CONSUMER(jLeU, FMT2) \
	CONSUMER(jGeU, FMT2) \
	CONSUMER(jLtF, FMT2) \
	CONSUMER(jGtF, FMT2) \
	CONSUMER(jLeF, FMT2) \
	CONSUMER(jGeF, FMT2) \
	CONSUMER(addI, FMT3) \
	CONSUMER(mulI, FMT3) \
	CONSUMER(subI, FMT3) \
	CONSUMER(divI, FMT3) \
	CONSUMER(mod, FMT3) \
	CONSUMER(shlI, FMT3) \
	CONSUMER(shrI, FMT3) \
	CONSUMER(shrU, FMT3) \
	CONSUMER(andI, FMT3) \
	CONSUMER(orI, FMT3) \
	CONSUMER(xorI, FMT3) \
	CONSUMER(addF, FMT3) \
	CONSUMER(mulF, FMT3) \
	CONSUMER(subF, FMT3) \
	CONSUMER(divF, FMT3) \
	CONSUMER(jump, FMT4) \
	CONSUMER(drop, FMT5) \
	CONSUMER(call, FMT5) \
	CONSUMER(ret, FMT5)

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

		static inline constexpr auto global(uint16_t index)
		{
			Reg ret;
			ret.kind = Kind::Global;
			ret.index = index;
			return ret;
		}
	};

	enum class Operation
	{
#define X(name, fmt) name,
		OPERATION_LIST(X)
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
		OPERATION_LIST(X)
#undef X
};

} //namespace prog

#endif /* INSTRUCTION_H_ */
