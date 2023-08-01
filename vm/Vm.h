#ifndef VM_H_
#define VM_H_

#include "object/Storage.h"
#include "program/Program.h"
#include "program/Function.h"

namespace vm {

class Vm
{
	obj::Storage& storage;
	const prog::Program &program;
	obj::Reference staticObject;

	class ExecutionState
	{
		obj::Reference frame = obj::null;
		uint32_t stackPointer = 0;
		uint32_t functionIndex = 0;
		decltype(prog::Function::code)::const_iterator isnIt, end;
		inline ExecutionState() = default;

	public:
		static inline ExecutionState enter(Vm& vm, uint32_t fnIdx, obj::Reference caller, std::vector<obj::Value> &args, size_t argCount);
		inline obj::Reference getCallerFrame(Vm& vm);

		inline bool fetch(prog::Instruction&);
		inline void jump(Vm& vm, uint32_t offset);

		inline obj::Value pop(Vm& vm);
		inline void push(Vm& vm, obj::Value value);

		inline obj::Value readLocal(Vm& vm, uint32_t offset);
		inline void writeLocal(Vm& vm, uint32_t offset, obj::Value value);

		inline void suspend(Vm& vm);
		static inline ExecutionState resume(Vm& vm, obj::Reference frame);
	};

public:
	Vm(obj::Storage& storage, const prog::Program &p);
	std::optional<obj::Value> run(std::vector<obj::Value> args);
};

} //namespace vm

#endif /* VM_H_ */
