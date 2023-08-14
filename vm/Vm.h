#ifndef VM_H_
#define VM_H_

#include "Storage.h"
#include "program/Program.h"

#include <vector>

namespace vm {

class Vm
{
	Storage& storage;
	const prog::Program &program;
	Reference staticObject;

	struct ExecutionState
	{
		Reference frame = null;
		uint32_t scalarStackPointer = 0;
		uint32_t referenceStackPointer = 0;
		uint32_t functionIndex = 0;
		decltype(prog::Function::code)::const_iterator isnIt, end;

		inline ExecutionState() = default;
	};

	inline ExecutionState enter(uint32_t fnIdx, Reference caller);
	inline Reference suspend(ExecutionState& es);
	inline ExecutionState resume(Reference frame);
	inline Reference getCallerFrame(ExecutionState& es);

	inline bool fetch(ExecutionState& es, prog::Instruction& isn);
	inline void jump(ExecutionState& es, uint32_t offset);

	inline Value reads(ExecutionState& es, prog::Instruction::Reg reg);
	inline Reference readr(ExecutionState& es, prog::Instruction::Reg reg);
	inline void writes(ExecutionState& es, prog::Instruction::Reg reg, Value value);
	inline void writer(ExecutionState& es, prog::Instruction::Reg reg, Reference ref);

	inline std::vector<Value> takes(ExecutionState& es, size_t n);
	inline std::vector<Reference> taker(ExecutionState& es, size_t n);
	inline void puts(ExecutionState& es, const std::vector<Value> & ss);
	inline void putr(ExecutionState& es, const std::vector<Reference> & rs);

	template<class C> inline void unary(ExecutionState& es, const prog::Instruction& isn, C&& c);
	template<class C> inline void conditional(ExecutionState& es, const prog::Instruction& isn, C&& c);
	template<class C> inline void binary(ExecutionState& es, const prog::Instruction& isn, C&& c);

public:
	Vm(Storage& storage, const prog::Program &p);
	std::pair<std::vector<Value>, std::vector<Reference>> run(std::vector<Value> sargs, std::vector<Reference> rargs);
};

} //namespace vm

#endif /* VM_H_ */
