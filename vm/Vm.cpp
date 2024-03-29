#include "Vm.h"

#include "Value.h"

#include <algorithm>

using namespace vm;

struct Frame
{
	struct Scalar
	{
		static constexpr auto scalarTosIndexOffset = 0;
		static constexpr auto referenceTosIndexOffset = 1;
		static constexpr auto stackOffset = 2;
		static constexpr auto extra = 2;
	};

	struct Reference
	{
		static constexpr auto callerFrameReferenceOffset = 0;
		static constexpr auto stackOffset = 1;
	};
};

inline Vm::ExecutionState Vm::enter(uint32_t fnIdx, Reference caller)
{
	assert(fnIdx < program.functions.size());
	const auto &fun = program.functions[fnIdx];

	Vm::ExecutionState ret;

	ret.frame = storage.create(prog::TypeInfo(
		/* base idx */   0,
		/* references */ Frame::Reference::stackOffset + fun.nRefs,
		/* scalars */    Frame::Scalar::stackOffset + fun.nScalars + Frame::Scalar::extra
	)),

	ret.functionIndex = fnIdx,
	ret.isnIt = fun.code.cbegin(),
	ret.end = fun.code.cend(),

	storage.writer(ret.frame, Frame::Reference::callerFrameReferenceOffset, caller);

	return ret;
}

inline Reference Vm::suspend(ExecutionState& es)
{
	const auto offset = es.isnIt - program.functions[es.functionIndex].code.cbegin();

	assert(es.scalarStackPointer < program.functions[es.functionIndex].nScalars);
	storage.writes(es.frame, Frame::Scalar::stackOffset + es.scalarStackPointer + 0, (int)offset);
	storage.writes(es.frame, Frame::Scalar::stackOffset + es.scalarStackPointer + 1, (int)es.functionIndex);
	storage.writes(es.frame, Frame::Scalar::scalarTosIndexOffset, (int)es.scalarStackPointer);
	storage.writes(es.frame, Frame::Scalar::referenceTosIndexOffset, (int)es.referenceStackPointer);

	return es.frame;
}

inline Vm::ExecutionState Vm::resume(Reference frame)
{
	Vm::ExecutionState ret;

	ret.frame = frame;
	ret.scalarStackPointer = (uint32_t)storage.reads(frame, Frame::Scalar::scalarTosIndexOffset).integer;
	ret.referenceStackPointer = (uint32_t)storage.reads(frame, Frame::Scalar::referenceTosIndexOffset).integer;
	const auto offset = storage.reads(ret.frame, Frame::Scalar::stackOffset + ret.scalarStackPointer + 0).integer;
	ret.functionIndex = storage.reads(ret.frame, Frame::Scalar::stackOffset + ret.scalarStackPointer + 1).integer;

	const auto& fun = program.functions[ret.functionIndex];
	ret.isnIt = fun.code.cbegin() + offset;
	ret.end = fun.code.cend();

	return ret;
}

inline Reference Vm::getCallerFrame(ExecutionState& es) {
	return storage.readr(es.frame, Frame::Reference::callerFrameReferenceOffset);
}

inline Value Vm::reads(ExecutionState& es, prog::Instruction::Reg reg)
{
	if(reg.kind == prog::Instruction::Reg::Kind::Tos)
	{
		assert(0 < es.scalarStackPointer);
		return storage.reads(es.frame, Frame::Scalar::stackOffset + --es.scalarStackPointer);
	}
	else if(reg.kind == prog::Instruction::Reg::Kind::Local)
	{
		assert(reg.index < es.scalarStackPointer);
		return storage.reads(es.frame, Frame::Scalar::stackOffset + reg.index);
	}
	else
	{
		assert(reg.index < program.types[0].nScalars);
		return storage.reads(staticObject, reg.index);
	}
}

inline Reference Vm::readr(ExecutionState& es, prog::Instruction::Reg reg)
{
	if(reg.kind == prog::Instruction::Reg::Kind::Tos)
	{
		assert(0 < es.referenceStackPointer);
		const auto ret = storage.readr(es.frame, Frame::Reference::stackOffset + --es.referenceStackPointer);
		storage.writer(es.frame, Frame::Reference::stackOffset + es.referenceStackPointer, null);
		return ret;
	}
	else if(reg.kind == prog::Instruction::Reg::Kind::Local)
	{
		assert(reg.index < es.referenceStackPointer);
		return storage.readr(es.frame, Frame::Reference::stackOffset + reg.index);
	}
	else
	{
		assert(reg.index < program.types[0].nReferences);
		return storage.readr(staticObject, reg.index);
	}
}

inline void Vm::writes(ExecutionState& es, prog::Instruction::Reg reg, Value value)
{
	if(reg.kind == prog::Instruction::Reg::Kind::Tos)
	{
		assert(es.scalarStackPointer < program.functions[es.functionIndex].nScalars);
		storage.writes(es.frame, Frame::Scalar::stackOffset + es.scalarStackPointer++, value);
	}
	else if(reg.kind == prog::Instruction::Reg::Kind::Local)
	{
		assert(reg.index < es.scalarStackPointer);
		storage.writes(es.frame, Frame::Scalar::stackOffset + reg.index, value);
	}
	else
	{
		assert(reg.index < program.types[0].nScalars);
		return storage.writes(staticObject, reg.index, value);
	}
}

inline void Vm::writer(ExecutionState& es, prog::Instruction::Reg reg, Reference value)
{
	if(reg.kind == prog::Instruction::Reg::Kind::Tos)
	{
		assert(es.referenceStackPointer < program.functions[es.functionIndex].nRefs);
		storage.writer(es.frame, Frame::Reference::stackOffset + es.referenceStackPointer++, value);
	}
	else if(reg.kind == prog::Instruction::Reg::Kind::Local)
	{
		assert(reg.index < es.referenceStackPointer);
		storage.writer(es.frame, Frame::Reference::stackOffset + reg.index, value);
	}
	else
	{
		assert(reg.index < program.types[0].nReferences);
		return storage.writer(staticObject, reg.index, value);
	}
}

inline std::vector<Value> Vm::takes(ExecutionState& es, size_t n)
{
	std::vector<Value> ret;

	assert(n <= es.scalarStackPointer);

	while(n--)
	{
		ret.push_back(reads(es, {}));
	}

	return ret;
}

inline std::vector<Reference> Vm::taker(ExecutionState& es, size_t n)
{
	std::vector<Reference> ret;

	assert(n <= es.referenceStackPointer);

	while(n--)
	{
		ret.push_back(readr(es, {}));
	}

	return ret;
}

inline void Vm::puts(ExecutionState& es, const std::vector<Value> & ss)
{
	assert(ss.size() + es.scalarStackPointer <= program.functions[es.functionIndex].nScalars);
	std::for_each(ss.rbegin(), ss.rend(), [this, &es](const auto& v){writes(es, {}, v);});
}

inline void Vm::putr(ExecutionState& es, const std::vector<Reference> & rs)
{
	assert(rs.size() + es.referenceStackPointer <= program.functions[es.functionIndex].nRefs);
	std::for_each(rs.rbegin(), rs.rend(), [this, &es](const auto& v){writer(es, {}, v);});
}

inline bool Vm::fetch(ExecutionState& es, prog::Instruction& isn)
{
	if(es.isnIt != es.end)
	{
		isn = *es.isnIt++;
		return true;
	}

	return false; // GCOV_EXCL_LINE
}

inline void Vm::jump(ExecutionState& es, uint32_t offset)
{
	assert(es.functionIndex < program.functions.size());
	const auto &fun = program.functions[es.functionIndex];

	assert(offset < fun.code.size());
	es.isnIt = fun.code.begin() + offset;
}

Vm::Vm(Storage& storage, const prog::Program &p): storage(storage), program(p)
{
	assert(!p.types.empty());
	staticObject = storage.create(p.types[0]);
}

template<class C> inline void Vm::unary(ExecutionState& es, const prog::Instruction& isn, C&& c) {
	this->writes(es, isn.x, c(this->reads(es, isn.y)));
}

template<class C> inline void Vm::conditional(ExecutionState& es, const prog::Instruction& isn, C&& c)
{
	const auto a = this->reads(es, isn.x);
	const auto b = this->reads(es, isn.y);

	if(c(a, b))
	{
		jump(es, isn.imm);
	}
}

template<class C> inline void Vm::binary(ExecutionState& es, const prog::Instruction& isn, C&& c)
{
	const auto a = this->reads(es, isn.y);
	const auto b = this->reads(es, isn.z);
	this->writes(es, isn.x, c(a, b));
}

std::pair<std::vector<Reference>, std::vector<Value>> Vm::run(std::vector<Reference> rargs, std::vector<Value> sargs)
{
	assert(!program.functions.empty());

	auto es = enter(0, staticObject);
	puts(es, sargs);
	putr(es, rargs);

	while(true)
	{
		prog::Instruction isn;
		auto fetchOk = fetch(es, isn);
		assert(fetchOk);

		switch(isn.op)
		{
		case prog::Instruction::Operation::lit:
			this->writes(es, isn.x, (int)isn.imm);
			break;
		case prog::Instruction::Operation::make:
			assert(isn.imm < program.types.size());
			this->writer(es, isn.x, isn.imm ? storage.create(program.types[isn.imm]) : null);
			break;
		case prog::Instruction::Operation::jNul:
			if(readr(es, isn.x) == null)
			{
				jump(es, isn.imm);
			}
			break;
		case prog::Instruction::Operation::jNnl:
			if(readr(es, isn.x) != null)
			{
				jump(es, isn.imm);
			}
			break;
		case prog::Instruction::Operation::movr:
			this->writer(es, isn.x, this->readr(es, isn.y));
			break;
		case prog::Instruction::Operation::mov:
			unary(es, isn, [](const auto& v){ return v; });
			break;
		case prog::Instruction::Operation::neg:
			unary(es, isn, [](const auto& v){ return ~v.integer; });
			break;
		case prog::Instruction::Operation::i2f:
			unary(es, isn, [](const auto& v){ return (float)(v.integer); });
			break;
		case prog::Instruction::Operation::f2i:
			unary(es, isn, [](const auto& v){ return (int)(v.floating); });
			break;
//		case prog::Instruction::Operation::x1i:
//			unary(es, isn, [](const auto& v){ return (int)(int32_t)((int8_t)v.integer); });
//			break;
//		case prog::Instruction::Operation::x1u:
//			unary(es, isn, [](const auto& v){ return (int)(uint32_t)((uint8_t)v.integer); });
//			break;
//		case prog::Instruction::Operation::x2i:
//			unary(es, isn, [](const auto& v){ return (int)(int32_t)((int16_t)v.integer); });
//			break;
//		case prog::Instruction::Operation::x2u:
//			unary(es, isn, [](const auto& v){ return (int)(uint32_t)((uint16_t)v.integer); });
//			break;
		case prog::Instruction::Operation::getr:
			this->writer(es, isn.x, storage.readr(this->readr(es, isn.y), isn.imm));
			break;
		case prog::Instruction::Operation::putr:
			storage.writer(this->readr(es, isn.y), isn.imm, this->readr(es, isn.x));
			break;
		case prog::Instruction::Operation::gets:
			this->writes(es, isn.x, storage.reads(this->readr(es, isn.y), isn.imm));
			break;
		case prog::Instruction::Operation::puts:
			storage.writes(this->readr(es, isn.y), isn.imm, this->reads(es, isn.x));
			break;
		case prog::Instruction::Operation::jEq:
			conditional(es, isn, [](const auto& a, const auto& b){ return a.integer == b.integer; });
			break;
		case prog::Instruction::Operation::jNe:
			conditional(es, isn, [](const auto& a, const auto& b){ return a.integer != b.integer; });
			break;
		case prog::Instruction::Operation::jLtI:
			conditional(es, isn, [](const auto& a, const auto& b){ return a.integer < b.integer; });
			break;
		case prog::Instruction::Operation::jGtI:
			conditional(es, isn, [](const auto& a, const auto& b){ return a.integer > b.integer; });
			break;
		case prog::Instruction::Operation::jLeI:
			conditional(es, isn, [](const auto& a, const auto& b){ return a.integer <= b.integer; });
			break;
		case prog::Instruction::Operation::jGeI:
			conditional(es, isn, [](const auto& a, const auto& b){ return a.integer >= b.integer; });
			break;
		case prog::Instruction::Operation::jLtU:
			conditional(es, isn, [](const auto& a, const auto& b){ return (uint32_t)a.integer < (uint32_t)b.integer; });break;
			break;
		case prog::Instruction::Operation::jGtU:
			conditional(es, isn, [](const auto& a, const auto& b){ return (uint32_t)a.integer > (uint32_t)b.integer; });break;
			break;
		case prog::Instruction::Operation::jLeU:
			conditional(es, isn, [](const auto& a, const auto& b){ return (uint32_t)a.integer <= (uint32_t)b.integer; });break;
			break;
		case prog::Instruction::Operation::jGeU:
			conditional(es, isn, [](const auto& a, const auto& b){ return (uint32_t)a.integer >= (uint32_t)b.integer; });break;
			break;
		case prog::Instruction::Operation::jLtF:
			conditional(es, isn, [](const auto& a, const auto& b){ return a.floating < b.floating; });
			break;
		case prog::Instruction::Operation::jGtF:
			conditional(es, isn, [](const auto& a, const auto& b){ return a.floating > b.floating; });
			break;
		case prog::Instruction::Operation::jLeF:
			conditional(es, isn, [](const auto& a, const auto& b){ return a.floating <= b.floating; });
			break;
		case prog::Instruction::Operation::jGeF:
			conditional(es, isn, [](const auto& a, const auto& b){ return a.floating >= b.floating; });
			break;
		case prog::Instruction::Operation::addI:
			binary(es, isn, [](const auto& a, const auto& b){ return a.integer + b.integer; });
			break;
		case prog::Instruction::Operation::mulI:
			binary(es, isn, [](const auto& a, const auto& b){ return a.integer * b.integer; });
			break;
		case prog::Instruction::Operation::subI:
			binary(es, isn, [](const auto& a, const auto& b){ return a.integer - b.integer; });
			break;
		case prog::Instruction::Operation::divI:
			binary(es, isn, [](const auto& a, const auto& b){ return a.integer / b.integer; });
			break;
		case prog::Instruction::Operation::mod:
			binary(es, isn, [](const auto& a, const auto& b){ return a.integer % b.integer; });
			break;
		case prog::Instruction::Operation::shlI:
			binary(es, isn, [](const auto& a, const auto& b){ return a.integer << b.integer; });
			break;
		case prog::Instruction::Operation::shrI:
			binary(es, isn, [](const auto& a, const auto& b){ return a.integer >> b.integer; });
			break;
		case prog::Instruction::Operation::shrU:
			binary(es, isn, [](const auto& a, const auto& b){ return (int)(((uint32_t)a.integer) >> b.integer); });
			break;
		case prog::Instruction::Operation::andI:
			binary(es, isn, [](const auto& a, const auto& b){ return a.integer & b.integer; });
			break;
		case prog::Instruction::Operation::orI:
			binary(es, isn, [](const auto& a, const auto& b){ return a.integer | b.integer; });
			break;
		case prog::Instruction::Operation::xorI:
			binary(es, isn, [](const auto& a, const auto& b){ return a.integer ^ b.integer; });
			break;
		case prog::Instruction::Operation::addF:
			binary(es, isn, [](const auto& a, const auto& b){ return a.floating + b.floating; });
			break;
		case prog::Instruction::Operation::mulF:
			binary(es, isn, [](const auto& a, const auto& b){ return a.floating * b.floating; });
			break;
		case prog::Instruction::Operation::subF:
			binary(es, isn, [](const auto& a, const auto& b){ return a.floating - b.floating; });
			break;
		case prog::Instruction::Operation::divF:
			binary(es, isn, [](const auto& a, const auto& b){ return a.floating / b.floating; });
			break;
		case prog::Instruction::Operation::jump:
			jump(es, isn.imm);
			break;
		case prog::Instruction::Operation::drop:
			taker(es, isn.imm);
			takes(es, isn.imm2);
			break;
		case prog::Instruction::Operation::call:
			{
				const auto calleeIdx = (uint32_t)reads(es, {}).integer;
				const auto rs = taker(es, isn.imm);
				const auto ss = takes(es, isn.imm2);
				es = enter(calleeIdx, suspend(es));
				putr(es, rs);
				puts(es, ss);
			}
			// TODO
			break;
		case prog::Instruction::Operation::ret:
			if(auto prevFrame = getCallerFrame(es); prevFrame != staticObject)
			{
				const auto rs = taker(es, isn.imm);
				const auto ss = takes(es, isn.imm2);
				es = resume(prevFrame);
				putr(es, rs);
				puts(es, ss);
			}
			else
			{
				return std::make_pair(taker(es, isn.imm), takes(es, isn.imm2));
			}
			break;
		}
	}

	return {};
}
