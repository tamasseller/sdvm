#ifndef CONECPT_BINARY_H_
#define CONECPT_BINARY_H_

#include <stdint.h>

enum class OpPrecedence: uint8_t
{
	CastCallMember,
	Unary,
	MultiplicativeBinary,
	AdditiveBinary,
	Shift,
	Relational,
	Equality,
	BitAnd,
	BitXor,
	BitOr,
	LogicAnd,
	LogicOr,
	Root
};

#define _ARITHMETIC_OPERATORS(CONSUMER) \
	CONSUMER (AddI, "+",  OpPrecedence::AdditiveBinary) \
	CONSUMER (MulI, "*",  OpPrecedence::MultiplicativeBinary) \
	CONSUMER (SubI, "-",  OpPrecedence::AdditiveBinary) \
	CONSUMER (DivI, "/",  OpPrecedence::MultiplicativeBinary) \
	CONSUMER (Mod,  "%",  OpPrecedence::MultiplicativeBinary) \
	CONSUMER (ShlI, "<<", OpPrecedence::Shift) \
	CONSUMER (ShrI, ">>", OpPrecedence::Shift) \
	CONSUMER (ShrU, ">>", OpPrecedence::Shift) \
	CONSUMER (AndI, "&",  OpPrecedence::BitAnd) \
	CONSUMER (OrI,  "|",  OpPrecedence::BitOr) \
	CONSUMER (XorI, "^",  OpPrecedence::BitXor) \
	CONSUMER (AddF, "+",  OpPrecedence::AdditiveBinary) \
	CONSUMER (MulF, "*",  OpPrecedence::MultiplicativeBinary) \
	CONSUMER (SubF, "-",  OpPrecedence::AdditiveBinary) \
	CONSUMER (DivF, "/",  OpPrecedence::MultiplicativeBinary) \

#define _CONDITIONAL_OPERATORS(CONSUMER) \
	CONSUMER (Eq,   "==", OpPrecedence::Equality) \
	CONSUMER (Ne,   "!=", OpPrecedence::Equality) \
	CONSUMER (LtI,  "<",  OpPrecedence::Relational) \
	CONSUMER (GtI,  ">",  OpPrecedence::Relational) \
	CONSUMER (LeI,  "<=", OpPrecedence::Relational) \
	CONSUMER (GeI,  ">=", OpPrecedence::Relational) \
	CONSUMER (LtU,  "<",  OpPrecedence::Relational) \
	CONSUMER (GtU,  ">",  OpPrecedence::Relational) \
	CONSUMER (LeU,  "<=", OpPrecedence::Relational) \
	CONSUMER (GeU,  ">=", OpPrecedence::Relational) \
	CONSUMER (LtF,  "<",  OpPrecedence::Relational) \
	CONSUMER (GtF,  ">",  OpPrecedence::Relational) \
	CONSUMER (LeF,  "<=", OpPrecedence::Relational) \
	CONSUMER (GeF,  ">=", OpPrecedence::Relational) \

#define _BOOL_PSEUDO_OPERATORS(CONSUMER) \
	CONSUMER (And,  "&&", OpPrecedence::LogicAnd) \
	CONSUMER (Or,   "||", OpPrecedence::LogicOr) \

#define _BINARY_OPERATORS(CONSUMER) \
	_ARITHMETIC_OPERATORS(CONSUMER) \
	_CONDITIONAL_OPERATORS(CONSUMER) \
	_BOOL_PSEUDO_OPERATORS(CONSUMER) \

#endif /* CONECPT_BINARY_H_ */
