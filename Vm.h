#ifndef VM_H_
#define VM_H_

#include "Storage.h"
#include "Program.h"

class Vm
{
	Storage& storage;
	const Program &program;
	const Storage::Ref staticObject;

	Storage::Ref createFrame(const Program::Function &f, Storage::Ref caller, std::vector<Storage::Value> &args, size_t argCount);

public:
	Vm(Storage& storage, const Program &p);
	std::optional<Storage::Value> run(std::vector<Storage::Value> args);
};

#endif /* VM_H_ */
