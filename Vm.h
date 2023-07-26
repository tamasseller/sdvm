#ifndef VM_H_
#define VM_H_

#include "Storage.h"
#include "Program.h"

class Vm
{
	Storage& storage;
	const Program &program;
	const Storage::Ref staticObject;

	class StackState
	{
		uint32_t stackPointer = 0;
		uint32_t refChainEnd = 0;

		inline StackState() = default;

	public:
		inline StackState(uint32_t stackPointer): stackPointer(stackPointer) {}

		inline Storage::Value pop(Storage& storage, Storage::Ref frame);
		inline void pushValue(Storage& storage, Storage::Ref frame, Storage::Value value);
		inline void pushReference(Storage& storage, Storage::Ref frame, Storage::Value value);

		inline void preGcFlush(Storage& storage, Storage::Ref frame);
		inline void store(Storage& storage, Storage::Ref frame);
		static inline StackState load(Storage& storage, Storage::Ref frame);
	};

	Storage::Ref createFrame(const Program::Function &f, Storage::Ref caller, std::vector<Storage::Value> &args, size_t argCount);

public:
	Vm(Storage& storage, const Program &p);
	std::optional<Storage::Value> run(std::vector<Storage::Value> args);
};

#endif /* VM_H_ */
