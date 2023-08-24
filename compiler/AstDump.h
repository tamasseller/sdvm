#ifndef COMPILER_ASTDUMP_H_
#define COMPILER_ASTDUMP_H_

#include "model/StatementTypes.h"

#include <map>
#include <sstream>
#include <algorithm>

namespace comp {


static inline const std::map<Binary::Operation, std::string> binaryOpString = {
	{Binary::Operation::AddI, "+"},
	{Binary::Operation::MulI, "*"},
	{Binary::Operation::SubI, "-"},
	{Binary::Operation::DivI, "/"},
	{Binary::Operation::Mod,  "%"},
	{Binary::Operation::ShlI, "<<"},
	{Binary::Operation::ShrI, ">>"},
	{Binary::Operation::ShrU, ">>"},
	{Binary::Operation::AndI, "&"},
	{Binary::Operation::OrI,  "|"},
	{Binary::Operation::XorI, "^"},
	{Binary::Operation::AddF, "+"},
	{Binary::Operation::MulF, "*"},
	{Binary::Operation::SubF, "-"},
	{Binary::Operation::DivF, "/"},
	{Binary::Operation::Eq,   "=="},
	{Binary::Operation::Ne,   "!="},
	{Binary::Operation::LtI,  "<"},
	{Binary::Operation::GtI,  ">"},
	{Binary::Operation::LeI,  "<="},
	{Binary::Operation::GeI,  ">="},
	{Binary::Operation::LtU,  "<"},
	{Binary::Operation::GtU,  ">"},
	{Binary::Operation::LeU,  "<="},
	{Binary::Operation::GeU,  ">="},
	{Binary::Operation::LtF,  "<"},
	{Binary::Operation::GtF,  ">"},
	{Binary::Operation::LeF,  "<="},
	{Binary::Operation::GeF,  ">="},
	{Binary::Operation::And,  "&&"},
	{Binary::Operation::Or,   "||"}
};

static inline const std::map<Unary::Operation, std::string> unaryOpString = {
	{Unary::Operation::Neg, "~"},
	{Unary::Operation::I2F, "(float)"},
	{Unary::Operation::F2I, "(int)"},
	{Unary::Operation::Not, "!"},
};

static inline std::string typeName(ValueType vt, const std::map<std::shared_ptr<Class>, size_t> &classIdxTable)
{
	if(vt.kind == TypeKind::Value)
	{
		switch(vt.primitiveType) {
			case PrimitiveType::Integer: return "int";
			case PrimitiveType::Floating: return "float";
			case PrimitiveType::Logical: return "logical";
			case PrimitiveType::Native: return "native";
			default: return "???";
		}
	}
	else
	{
		assert(vt.kind == TypeKind::Reference);
		return "c" + std::to_string(classIdxTable.find(vt.referenceType)->second);
	}
}

std::string repeatStr(size_t num, const std::string& input = "  ")
{
    std::string ret;
    ret.reserve(input.size() * num);

    while (num--) {
    	ret += input;
    }

    return ret;
}

static inline std::string dumpExpressionAst(const std::map<std::shared_ptr<Class>, size_t> &classIdxTab, const std::map<std::shared_ptr<Function>, size_t> &fnIdxTab, std::map<const Local*, size_t> &locals, std::shared_ptr<RValue> val)
{
	std::string ret;

	val->accept(overloaded
	{
		[&](const Call& v)
		{
			std::stringstream ss;
			ss << "f" << fnIdxTab.find(v.fn)->second << "(";

			const char* sep = "";
			for(const auto &a: v.args) {
				ss << sep << dumpExpressionAst(classIdxTab, fnIdxTab, locals, a);
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
			ret = unaryOpString.find(v.op)->second + dumpExpressionAst(classIdxTab, fnIdxTab, locals, v.arg);
		},
		[&](const Binary& v)
		{
			ret = "(" + dumpExpressionAst(classIdxTab, fnIdxTab, locals, v.first) + " " + binaryOpString.find(v.op)->second + " " + dumpExpressionAst(classIdxTab, fnIdxTab, locals, v.second) + ")";
		},
		[&](const Create& v)
		{
			ret = "new(c" + std::to_string(classIdxTab.find(v.type)->second) + ")";
		},
		[&](const Global& v)
		{
			ret = "c" + std::to_string(classIdxTab.find(v.field.type)->second) + "::s" + std::to_string(v.field.index);
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
			ret = dumpExpressionAst(classIdxTab, fnIdxTab, locals, v.condition) + " ? " + dumpExpressionAst(classIdxTab, fnIdxTab, locals, v.then) + " : " + dumpExpressionAst(classIdxTab, fnIdxTab, locals, v.otherwise);
		},
		[&](const Argument& v)
		{
			ret = std::string("a") + std::to_string(v.idx);
		},
		[&](const Dereference& v)
		{
			ret = dumpExpressionAst(classIdxTab, fnIdxTab, locals, v.object) + "->f" + std::to_string(v.field.index);
		}
	});

	return ret;
}

static inline void dumpStatementAst(const std::map<std::shared_ptr<Class>, size_t> &classIdxTab, const std::map<std::shared_ptr<Function>, size_t> &fnIdxTab, std::map<const Local*, size_t> &locals, std::stringstream &ss, std::shared_ptr<Statement> stmt, int indent)
{
	stmt->accept(overloaded
	{
		[&](const ExpressionStatement& v)
		{
			ss << repeatStr(indent) << dumpExpressionAst(classIdxTab, fnIdxTab, locals, v.val) << std::endl;
		},
		[&](const Conditional& v)
		{
			ss << repeatStr(indent) << "if " << dumpExpressionAst(classIdxTab, fnIdxTab, locals, v.condition) << std::endl;
			dumpStatementAst(classIdxTab, fnIdxTab, locals, ss, v.then, indent);

			if(!v.otherwise->stmts.empty())
			{
				ss << repeatStr(indent) << "else" << std::endl;
				dumpStatementAst(classIdxTab, fnIdxTab, locals, ss, v.otherwise, indent);
			}
		},
		[&](const Declaration& v)
		{
			locals.insert({v.local.get(), locals.size()});
			ss << repeatStr(indent) << typeName(v.local->type, classIdxTab) << " l" << locals[v.local.get()] <<  " = " << dumpExpressionAst(classIdxTab, fnIdxTab, locals, v.initializer) << std::endl;
		},
		[&](const Continue& v)
		{
			ss << repeatStr(indent) << "continue" << std::endl;
		},
		[&](const Return& v)
		{
			if(v.value.empty())
			{
				ss << repeatStr(indent) << "return" << std::endl;
			}
			else
			{
				assert(v.value.size() == 1);
				ss << repeatStr(indent) << "return " << dumpExpressionAst(classIdxTab, fnIdxTab, locals, v.value[0]) << std::endl;
			}
		},
		[&](const Block& v)
		{
			ss << repeatStr(indent) << "{" << std::endl;
			std::for_each(v.stmts.begin(), v.stmts.end(), [&](const auto& s){dumpStatementAst(classIdxTab, fnIdxTab, locals, ss, s, indent + 1);});
			ss << repeatStr(indent) << "}" << std::endl;
		},
		[&](const Break& v)
		{
			ss << repeatStr(indent) << "break" << std::endl;
		},
		[&](const Loop& v)
		{
			ss << repeatStr(indent) << "loop" << std::endl;
			dumpStatementAst(classIdxTab, fnIdxTab, locals, ss, v.body, indent);
		},
		[&](const Set& v)
		{
			ss << repeatStr(indent) << dumpExpressionAst(classIdxTab, fnIdxTab, locals, v.target) << " = " << dumpExpressionAst(classIdxTab, fnIdxTab, locals, v.value) << std::endl;
		}
	});

}

static inline std::string dumpFunctionAst(const std::map<std::shared_ptr<Class>, size_t> &classIdxTab, const std::map<std::shared_ptr<Function>, size_t> &fnIdxTab, std::shared_ptr<Function> fn)
{
	std::stringstream ss;
	std::map<const Local*, size_t> locals;
	ss << (fn->ret.empty() ? "void" : typeName(fn->ret[0], classIdxTab));
	ss << " f" << fnIdxTab.find(fn)->second << "(";

	const char* sep = "";
	for(auto i = 0u; i < fn->args.size(); i++) {
		ss << sep << typeName(fn->args[i], classIdxTab) << " a" << i;
		sep = ", ";
	}

	ss << ")" << std::endl;
	dumpStatementAst(classIdxTab, fnIdxTab, locals, ss, fn->body, 0);

	const auto ret = ss.str();
	return ret;
}

static inline std::string dumpClassAst(const std::map<std::shared_ptr<Class>, size_t> &classIdxTab, std::shared_ptr<Class> cl)
{
	std::stringstream ss;
	ss << "class c" << classIdxTab.find(cl)->second << std::endl;
	ss << "{" << std::endl;

	for(auto i = 0u; i < cl->staticTypes.size(); i++) {
		ss << "  " << typeName(cl->staticTypes[i], classIdxTab) << " s" << i << std::endl;
	}

	for(auto i = 0u; i < cl->fieldTypes.size(); i++) {
		ss << "  " << typeName(cl->fieldTypes[i], classIdxTab) << " f" << i << std::endl;
	}

	ss << "}" << std::endl;
	const auto ret = ss.str();
	return ret;
}

} // namespace comp

#endif /* COMPILER_ASTDUMP_H_ */
