#include "BasicBlock.h"

#include "Operations.h"
#include "Terminations.h"

#include "Overloaded.h"

#include "compiler/common/Annotations.h"

#include "concept/Binary.h"

#include "assert.h"

#include <set>
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
#define X(n, s, ...) {Binary::Op:: n, s},
		_ARITHMETIC_OPERATORS(X)
#undef X
};

void BasicBlock::traverse(std::shared_ptr<BasicBlock> entry, std::function<void(std::shared_ptr<BasicBlock>)> c)
{
	std::set<std::shared_ptr<BasicBlock>> toDo{entry}, done;

	while(!toDo.empty())
	{
		const auto current = *toDo.begin();
		toDo.erase(current);
		if(done.insert(current).second)
		{
			c(current);

			current->termination->accept(overloaded
			{
				[&](const Leave &v){},
				[&](const Always &v)
				{
					toDo.insert(v.continuation);
				},
				[&](const Conditional &v)
				{
					toDo.insert(v.then);
					toDo.insert(v.otherwise);
				},
			});
		}
	}
}

std::string BasicBlock::DumpContext::nameOf(const std::shared_ptr<Temporary> &t)
{
	if(t->isConstant())
	{
		const auto c = std::dynamic_pointer_cast<Constant>(t);

		assert(t->type.kind == ast::TypeKind::Value);

		switch(t->type.primitiveType)
		{
			case ast::PrimitiveType::Integer: return std::to_string(c->value);
			case ast::PrimitiveType::Floating: return std::to_string(*reinterpret_cast<const float*>(&c->value));
			case ast::PrimitiveType::Logical: return c->value ? "true" : "false";
			default: break;
		}
	}
	else
	{
		const auto v = std::dynamic_pointer_cast<Variable>(t);
		const auto it = ts.find(v);
		return "t" + std::to_string((it != ts.end()) ? it->second : ts.insert({v, ts.size()}).first->second);
	}

	return "<< native >>";
}

std::string BasicBlock::dump(ast::ProgramObjectSet& gi, DumpContext& dc) const
{
	std::stringstream ss;

	for(const std::shared_ptr<Operation> &o: code)
	{
		o->accept(overloaded
		{
			[&](const Copy& v) {ss << dc.nameOf(v.target) << " ← " << dc.nameOf(v.source);},
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

		for(const std::shared_ptr<Annotation> &a: o->annotations)
		{
			a->accept(overloaded{
				[&](const IrComment& v) {
					ss << " /* " << v.genText(dc) << " */";
				}
			});
		}

		ss << "\\n";
	}

	for(const std::shared_ptr<Annotation> &a: annotations)
	{
		a->accept(overloaded{
			[&](const IrComment& v) {
				ss << "/* " << v.genText(dc) << " */\\n";
			}
		});
	}

	const auto ret = ss.str();
	return ret;
}
