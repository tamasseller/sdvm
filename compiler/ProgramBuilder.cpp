#include "ProgramBuilder.h"

#include <map>
#include <algorithm>

using namespace comp;

ProgramBuilder::ProgramBuilder(): globals(Class::make()) {}

prog::Program ProgramBuilder::compile(Function entry)
{
	prog::Program ret;

	std::vector<std::shared_ptr<FunctionDesc>> functions{entry.data};

	// TODO iterate call tree and collect functions

	std::vector<std::shared_ptr<Class>> classes{globals.data};

	for(const auto &f: functions) {
		classes.push_back(f->locals.data);
	}

	// TODO iterate call tree and collect classes

	std::map<std::shared_ptr<Class>, size_t> classIdxTable;
	std::transform(classes.begin(), classes.end(), std::inserter(classIdxTable, classIdxTable.end()), [idx{0u}](const std::shared_ptr<Class>& c) mutable {
		return std::make_pair(c, idx++);
	});

	std::transform(classes.begin(), classes.end(), std::back_inserter(ret.types), [&classIdxTable](const std::shared_ptr<Class>& c){
		return obj::TypeInfo {
			.baseIdx = c->base ? classIdxTable[c->base] : 0,
			.length = c->size(),
			.refOffs = c->getRefOffs()
		};
	});

	for(const auto& f: functions)
	{
		prog::Function fn;
		fn.opStackSize = 0;
		fn.frameTypeIndex = classIdxTable[f->locals.data],

		std::for_each(f->code.begin(), f->code.end(), [&fn](const Line& line)
		{
			if(auto isn = line(); isn.has_value())
			{
				fn.code.push_back(isn.value());
			}
		});

		ret.functions.push_back(fn);
	}

	return ret;
}

/*
struct CodeWriter
{
	int stackDepth = 0, maxStackDepth = 0;
	bool hasCall = false;
	std::vector<prog::Instruction> code;

	void write(prog::Instruction isn);
};*/

/*
prog::Function FunctionBuilder::operator()()
{
	CodeWriter cw;

	for(const auto &c: code) {
		c(cw);
	}

	const auto opstackSize = cw.maxStackDepth + (cw.hasCall ? prog::Frame::callerStackExtra : 0);
	const auto opstackOffset = locals->size();

	for(int i = 0; i < opstackSize; i++)
	{
		locals->addField(ValueType::native());
	}

	return prog::Function
	{
		.frame = prog::Frame {
			.opStackOffset = opstackOffset,
			.frameTypeIndex = locals.idx
		},
		.code = std::move(cw.code)
	};
}

void FunctionBuilder::setLocal(const Class::FieldHandle &l, const RValue& b)
{
	// TODO check assignability

	code.push_back([this, l, b](CodeWriter& cw){
		b.manifest(cw);
		cw.write(prog::Instruction::writeLocal(pb.getFieldOffset(l)));
	});
}

RValue FunctionBuilder::create(Handle<Class> t)
{
	return RValue {
		.type = ValueType::reference(t.idx),
		.manifest = [t](CodeWriter& cw){
			cw.write(prog::Instruction::newObject(t.idx));
		}
	};
}
*/

/*
void CodeWriter::write(prog::Instruction isn)
{
	hasCall = hasCall || isn.op == prog::Instruction::Operation::Call || isn.op == prog::Instruction::Operation::CallV;
	stackDepth += isn.stackBalance();
	assert(0 <= stackDepth);
	maxStackDepth = std::max(maxStackDepth, stackDepth);
	code.push_back(isn);
}
*/

/*
uint32_t ProgramBuilder::getFieldOffset(Class::FieldHandle f) const
{
	auto baseSize = 0u;
	for(auto idx = f.typeIdx; types[f.typeIdx]->baseIdx; idx = *types[f.typeIdx]->baseIdx)
	{
		baseSize += types[idx]->size();
	}

	return baseSize + f.fieldIdx;
}
*/
