#ifndef VM_H_
#define VM_H_

#include "object/Storage.h"
#include "program/Program.h"

namespace vm {

class Vm
{
	obj::Storage& storage;
	const prog::Program &program;
	const obj::Reference staticObject;

	class StackState
	{
		uint32_t stackPointer = 0;
		uint32_t refChainEnd = 0;

		inline StackState() = default;

	public:
		inline StackState(uint32_t stackPointer): stackPointer(stackPointer) {}

		inline obj::Value pop(obj::Storage& storage, obj::Reference frame);
		inline void pushValue(obj::Storage& storage, obj::Reference frame, obj::Value value);
		inline void pushReference(obj::Storage& storage, obj::Reference frame, obj::Value value);

		inline void preGcFlush(obj::Storage& storage, obj::Reference frame);
		inline void store(obj::Storage& storage, obj::Reference frame);
		static inline StackState load(obj::Storage& storage, obj::Reference frame);
	};

	obj::Reference createFrame(const prog::Function &f, obj::Reference caller, std::vector<obj::Value> &args, size_t argCount);

public:
	Vm(obj::Storage& storage, const prog::Program &p);
	std::optional<obj::Value> run(std::vector<obj::Value> args);
};

} //namespace vm

#endif /* VM_H_ */
