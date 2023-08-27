#include "Function.h"

#include "compiler/ast/Values.h"
#include "compiler/ast/Statements.h"

#include "Statements.h"
#include "ProgramObjectSet.h"

#include "Overloaded.h"

#include <map>
#include <sstream>
#include <algorithm>

using namespace comp;
using namespace comp::ast;

Function::Function(std::vector<ValueType> ret, std::vector<ValueType> args):
	ret(ret), args(args), body(std::make_shared<Block>()) {}

std::string Function::getReferenceForDump(const ProgramObjectSet& gi) const {
	return std::string("f") + std::to_string(gi.getFunctionIndex(this));
}
const auto nIndentSpaces = 4;

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

static inline const std::map<Binary::Operation, std::pair<std::string, OpPrecedence>> binaryOps = {
	{Binary::Operation::AddI, {"+",  OpPrecedence::AdditiveBinary}},
	{Binary::Operation::MulI, {"*",  OpPrecedence::MultiplicativeBinary}},
	{Binary::Operation::SubI, {"-",  OpPrecedence::AdditiveBinary}},
	{Binary::Operation::DivI, {"/",  OpPrecedence::MultiplicativeBinary}},
	{Binary::Operation::Mod,  {"%",  OpPrecedence::MultiplicativeBinary}},
	{Binary::Operation::ShlI, {"<<", OpPrecedence::Shift}},
	{Binary::Operation::ShrI, {">>", OpPrecedence::Shift}},
	{Binary::Operation::ShrU, {">>", OpPrecedence::Shift}},
	{Binary::Operation::AndI, {"&",  OpPrecedence::BitAnd}},
	{Binary::Operation::OrI,  {"|",  OpPrecedence::BitOr}},
	{Binary::Operation::XorI, {"^",  OpPrecedence::BitXor}},
	{Binary::Operation::AddF, {"+",  OpPrecedence::AdditiveBinary}},
	{Binary::Operation::MulF, {"*",  OpPrecedence::MultiplicativeBinary}},
	{Binary::Operation::SubF, {"-",  OpPrecedence::AdditiveBinary}},
	{Binary::Operation::DivF, {"/",  OpPrecedence::MultiplicativeBinary}},
	{Binary::Operation::Eq,   {"==", OpPrecedence::Equality}},
	{Binary::Operation::Ne,   {"!=", OpPrecedence::Equality}},
	{Binary::Operation::LtI,  {"<",  OpPrecedence::Relational}},
	{Binary::Operation::GtI,  {">",  OpPrecedence::Relational}},
	{Binary::Operation::LeI,  {"<=", OpPrecedence::Relational}},
	{Binary::Operation::GeI,  {">=", OpPrecedence::Relational}},
	{Binary::Operation::LtU,  {"<",  OpPrecedence::Relational}},
	{Binary::Operation::GtU,  {">",  OpPrecedence::Relational}},
	{Binary::Operation::LeU,  {"<=", OpPrecedence::Relational}},
	{Binary::Operation::GeU,  {">=", OpPrecedence::Relational}},
	{Binary::Operation::LtF,  {"<",  OpPrecedence::Relational}},
	{Binary::Operation::GtF,  {">",  OpPrecedence::Relational}},
	{Binary::Operation::LeF,  {"<=", OpPrecedence::Relational}},
	{Binary::Operation::GeF,  {">=", OpPrecedence::Relational}},
	{Binary::Operation::And,  {"&&", OpPrecedence::LogicAnd}},
	{Binary::Operation::Or,   {"||", OpPrecedence::LogicOr}}
};

static inline const std::map<Unary::Operation, std::string> unaryOps = {
	{Unary::Operation::Neg, "~"},
	{Unary::Operation::I2F, "(float)"},
	{Unary::Operation::F2I, "(int)"},
	{Unary::Operation::Not, "!"},
};

static inline std::string dumpExpressionAst(const ProgramObjectSet& gi, std::map<const Local*, size_t> &locals, std::shared_ptr<const RValue> val, OpPrecedence prevPrec)
{
	std::string ret;

	val->accept(overloaded
	{
		[&](const Set& v)
		{
			ret = dumpExpressionAst(gi, locals, v.target, OpPrecedence::Root) + " = " +
					dumpExpressionAst(gi, locals, v.value, OpPrecedence::Root);
		},
		[&](const Call& v)
		{
			std::stringstream ss;
			ss << v.fn->getReferenceForDump(gi) << "(";

			const char* sep = "";
			for(const auto &a: v.args) {
				ss << sep << dumpExpressionAst(gi, locals, a, OpPrecedence::Root);
				sep = ", ";
			}

			ss << ")";
			ret = ss.str();
		},
		[&](const Local& v)
		{
			ret = std::string("l") + std::to_string(locals[&v]);
		},
		[&](const Unary& v)
		{
			ret = unaryOps.find(v.op)->second + dumpExpressionAst(gi, locals, v.arg, OpPrecedence::Unary);
		},
		[&](const Binary& v)
		{
			const auto &info = binaryOps.find(v.op)->second;
			const auto prec = info.second;
			ret = dumpExpressionAst(gi, locals, v.first, prec) + " " + info.first  + " " + dumpExpressionAst(gi, locals, v.second, prec);

			if(prevPrec < prec)
			{
				ret = "(" + ret + ")";
			}
		},
		[&](const Create& v)
		{
			ret = v.type ? "new " + v.type->getReferenceForDump(gi) : "nullptr";
		},
		[&](const Global& v)
		{
			ret = v.field.type->getReferenceForDump(gi) + "::s" + std::to_string(v.field.index);
		},
		[&](const Literal& v)
		{
			switch(v.type.primitiveType)
			{
			case PrimitiveType::Integer:
				ret = std::to_string(v.integer);
				break;
			case PrimitiveType::Floating:
				ret = std::to_string(v.floating) + "f";
				break;
			case PrimitiveType::Logical:
				ret = v.logical ? "true" : "false";
				break;
			case PrimitiveType::Native:
				ret = "<native>";
				break;
			}
		},
		[&](const Ternary& v)
		{
			ret = dumpExpressionAst(gi, locals, v.condition, OpPrecedence::Root) + " ? " +
					dumpExpressionAst(gi, locals, v.then, OpPrecedence::Root) + " : " +
					dumpExpressionAst(gi, locals, v.otherwise, OpPrecedence::Root);
		},
		[&](const Argument& v)
		{
			ret = std::string("a") + std::to_string(v.idx);
		},
		[&](const Dereference& v)
		{
			ret = dumpExpressionAst(gi, locals, v.object, OpPrecedence::CastCallMember) + "->f" + std::to_string(v.field.index);
		}
	});

	return ret;
}

static inline void dumpStatementAst(const ProgramObjectSet& gi, std::map<const Local*, size_t> &locals, std::stringstream &ss, std::shared_ptr<Statement> stmt, int indent)
{
	stmt->accept(overloaded
	{
		[&](const ExpressionStatement& v)
		{
			ss << std::string(nIndentSpaces * indent, ' ') << dumpExpressionAst(gi, locals, v.val, OpPrecedence::Root) << ";" << std::endl;
		},
		[&](const Conditional& v)
		{
			ss << std::string(nIndentSpaces * indent, ' ') << "if(" << dumpExpressionAst(gi, locals, v.condition, OpPrecedence::Root) << ")" << std::endl;
			dumpStatementAst(gi, locals, ss, v.then, indent);

			if(!v.otherwise->stmts.empty())
			{
				ss << std::string(nIndentSpaces * indent, ' ') << "else" << std::endl;
				dumpStatementAst(gi, locals, ss, v.otherwise, indent);
			}
		},
		[&](const Declaration& v)
		{
			locals.insert({v.local.get(), locals.size()});
			ss << std::string(nIndentSpaces * indent, ' ') << v.local->type.getReferenceForDump(gi)
				<< " l" << locals[v.local.get()] <<  " = "
				<< dumpExpressionAst(gi, locals, v.initializer, OpPrecedence::Root) << ";" << std::endl;
		},
		[&](const Continue& v)
		{
			ss << std::string(nIndentSpaces * indent, ' ') << "continue;" << std::endl;
		},
		[&](const Return& v)
		{
			if(v.value.empty())
			{
				ss << std::string(nIndentSpaces * indent, ' ') << "return;" << std::endl;
			}
			else
			{
				assert(v.value.size() == 1);
				ss << std::string(nIndentSpaces * indent, ' ') << "return "
					<< dumpExpressionAst(gi, locals, v.value[0], OpPrecedence::Root) << ";" << std::endl;
			}
		},
		[&](const Block& v)
		{
			ss << std::string(nIndentSpaces * indent, ' ') << "{" << std::endl;

			for(const auto& s: v.stmts)
			{
				dumpStatementAst(gi, locals, ss, s, indent + 1);
			}

			ss << std::string(nIndentSpaces * indent, ' ') << "}" << std::endl;
		},
		[&](const Break& v)
		{
			ss << std::string(nIndentSpaces * indent, ' ') << "break;" << std::endl;
		},
		[&](const Loop& v)
		{
			ss << std::string(nIndentSpaces * indent, ' ') << "while(true)" << std::endl;
			dumpStatementAst(gi, locals, ss, v.body, indent);
		}
	});
}

static inline void writeFunctionHeader(const ProgramObjectSet& gi, std::stringstream &ss, const Function* fn)
{
	ss << (fn->ret.empty() ? "void" : fn->ret[0].getReferenceForDump(gi));
	ss << " " << fn->getReferenceForDump(gi) << "(";

	const char* sep = "";
	for(auto i = 0u; i < fn->args.size(); i++) {
		ss << sep << fn->args[i].getReferenceForDump(gi) << " a" << i;
		sep = ", ";
	}

	ss << ")" << std::endl;
}

std::string Function::dump(const ProgramObjectSet& gi) const
{
	std::stringstream ss;
	std::map<const Local*, size_t> locals;
	writeFunctionHeader(gi, ss, this);
	dumpStatementAst(gi, locals, ss, body, 0);
	const auto ret = ss.str();
	return ret;
}
