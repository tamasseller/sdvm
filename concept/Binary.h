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
	CONSUMER (AddI, "+",  OpPrecedence::AdditiveBinary,       integer) \
	CONSUMER (MulI, "*",  OpPrecedence::MultiplicativeBinary, integer) \
	CONSUMER (SubI, "-",  OpPrecedence::AdditiveBinary,       integer) \
	CONSUMER (DivI, "/",  OpPrecedence::MultiplicativeBinary, integer) \
	CONSUMER (Mod,  "%",  OpPrecedence::MultiplicativeBinary, integer) \
	CONSUMER (ShlI, "<<", OpPrecedence::Shift,                integer) \
	CONSUMER (ShrI, ">>", OpPrecedence::Shift,                integer) \
	CONSUMER (ShrU, ">>", OpPrecedence::Shift,                integer) \
	CONSUMER (AndI, "&",  OpPrecedence::BitAnd,               integer) \
	CONSUMER (OrI,  "|",  OpPrecedence::BitOr,                integer) \
	CONSUMER (XorI, "^",  OpPrecedence::BitXor,               integer) \
	CONSUMER (AddF, "+",  OpPrecedence::AdditiveBinary,       floating) \
	CONSUMER (MulF, "*",  OpPrecedence::MultiplicativeBinary, floating) \
	CONSUMER (SubF, "-",  OpPrecedence::AdditiveBinary,       floating) \
	CONSUMER (DivF, "/",  OpPrecedence::MultiplicativeBinary, floating) \

#define _CONDITIONAL_OPERATORS(CONSUMER) \
	CONSUMER (Eq,   "==", OpPrecedence::Equality,   logical) \
	CONSUMER (Ne,   "!=", OpPrecedence::Equality,   logical) \
	CONSUMER (LtI,  "<",  OpPrecedence::Relational, logical) \
	CONSUMER (GtI,  ">",  OpPrecedence::Relational, logical) \
	CONSUMER (LeI,  "<=", OpPrecedence::Relational, logical) \
	CONSUMER (GeI,  ">=", OpPrecedence::Relational, logical) \
	CONSUMER (LtU,  "<",  OpPrecedence::Relational, logical) \
	CONSUMER (GtU,  ">",  OpPrecedence::Relational, logical) \
	CONSUMER (LeU,  "<=", OpPrecedence::Relational, logical) \
	CONSUMER (GeU,  ">=", OpPrecedence::Relational, logical) \
	CONSUMER (LtF,  "<",  OpPrecedence::Relational, logical) \
	CONSUMER (GtF,  ">",  OpPrecedence::Relational, logical) \
	CONSUMER (LeF,  "<=", OpPrecedence::Relational, logical) \
	CONSUMER (GeF,  ">=", OpPrecedence::Relational, logical) \

#define _BOOL_PSEUDO_OPERATORS(CONSUMER) \
	CONSUMER (And,  "&&", OpPrecedence::LogicAnd, logical) \
	CONSUMER (Or,   "||", OpPrecedence::LogicOr,  logical) \

#define _BINARY_OPERATORS(CONSUMER) \
	_ARITHMETIC_OPERATORS(CONSUMER) \
	_CONDITIONAL_OPERATORS(CONSUMER) \
	_BOOL_PSEUDO_OPERATORS(CONSUMER) \

#endif /* CONECPT_BINARY_H_ */
