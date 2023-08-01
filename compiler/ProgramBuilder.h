#ifndef PROGRAMBUILDER_H_
#define PROGRAMBUILDER_H_

#include "ClassBuilder.h"
#include "FunctionBuilder.h"

#include "program/Instruction.h"
#include "program/Program.h"

#include <optional>
#include <memory>

namespace comp {

class ProgramBuilder
{
	friend ClassBuilder;
	friend FunctionBuilder;

	std::vector<std::unique_ptr<ClassBuilder>> types;
	std::vector<std::unique_ptr<FunctionBuilder>> functions;

	Handle<ClassBuilder> globalBuilder;

	uint32_t getFieldOffset(ClassBuilder::FieldHandle) const;
public:
	ProgramBuilder();

	using Function = Handle<FunctionBuilder>;
	Function fun(std::optional<ValueType> ret, std::vector<ValueType> args);

	using Class = Handle<ClassBuilder>;
	Class type(std::optional<Handle<ClassBuilder>> base = {}, bool isFrame = false);

	template<class... Args>
	auto addGlobal(Args&&... args) {
		return globalBuilder->addField(std::forward<Args>(args)...);
	}

	prog::Program operator()();
};

} // namespace comp

#endif /* PROGRAMBUILDER_H_ */
