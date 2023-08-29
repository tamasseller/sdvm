#include "BasicBlock.h"

#include "Operations.h"
#include "Terminations.h"

#include "Overloaded.h"

#include "assert.h"

#include <sstream>
#include <algorithm>

using namespace comp;
using namespace comp::ir;

static inline const std::map<Unary::Op, std::string> unaryOp =
{
	{Unary::Op::Neg, "~"},
	{Unary::Op::I2F, "(int)"},
	{Unary::Op::F2I, "(float)"},
	{Unary::Op::Not, "!"},
};

static inline const std::map<Binary::Op, std::string> binaryOp =
{
	{Binary::Op::AddI, "+"},
	{Binary::Op::MulI, "*"},
	{Binary::Op::SubI, "-"},
	{Binary::Op::DivI, "/"},
	{Binary::Op::Mod,  "%"},
	{Binary::Op::ShlI, "<<"},
	{Binary::Op::ShrI, ">>"},
	{Binary::Op::ShrU, ">>>"},
	{Binary::Op::AndI, "&"},
	{Binary::Op::OrI,  "|"},
	{Binary::Op::XorI, "^"},
	{Binary::Op::AddF, "+"},
	{Binary::Op::MulF, "*"},
	{Binary::Op::SubF, "-"},
	{Binary::Op::DivF, "/"},
};

std::string BasicBlock::dump(ast::ProgramObjectSet& gi, DumpContext& dc) const
{
	std::stringstream ss;

	for(const auto &o: code)
	{
		o->accept(overloaded
		{
			[&](const Copy& v) {ss << dc.nameOf(v.target) << " ← " << dc.nameOf(v.source);},
			[&](const Literal& v) {ss << dc.nameOf(v.target) << " ← " << v.integer;},
			[&](const Unary& v) {ss << dc.nameOf(v.target) << " ← " << unaryOp.find(v.op)->second << dc.nameOf(v.source);},
			[&](const Create& v) {ss << dc.nameOf(v.target) << " ← new " << v.type->getReferenceForDump(gi);},
			[&](const LoadField& v) {ss << dc.nameOf(v.target) << " ← " << dc.nameOf(v.object) << "." << v.field.getReferenceForDump(gi);},
			[&](const StoreField& v) {ss << dc.nameOf(v.source) << " → " << dc.nameOf(v.object) << "." << v.field.getReferenceForDump(gi);},
			[&](const LoadGlobal& v) {ss << dc.nameOf(v.target) << " ← " << v.field.getReferenceForDump(gi);},
			[&](const StoreGlobal& v) {ss << dc.nameOf(v.source) << " → " << v.field.getReferenceForDump(gi);},
			[&](const Binary& v) {ss << dc.nameOf(v.target) << " ← " << dc.nameOf(v.first) << " " << binaryOp.find(v.op)->second << " " << dc.nameOf(v.second);},
			[&](const Call& v)
			{
				if(!v.ret.empty())
				{
					assert(v.ret.size() == 1);
					ss << dc.nameOf(v.ret[0]) << " ← ";
				}
				ss << v.fn->getReferenceForDump(gi) << "(";

				for(const auto& a: v.arg)
				{
					ss << dc.nameOf(a);
				}

				ss << ")";
			}
		});

		ss << "\\n";
	}

	const auto ret = ss.str();
	return ret;
}
