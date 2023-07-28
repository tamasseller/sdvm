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
	friend FunctionBuilder;

	std::vector<std::unique_ptr<ClassBuilder>> types;
	std::vector<std::unique_ptr<FunctionBuilder>> functions;

public:
	template<class Target>
	class Handle
	{
		std::vector<std::unique_ptr<Target>> &target;
		friend ProgramBuilder;

		inline Handle(std::vector<std::unique_ptr<Target>> &target, const size_t idx): target(target), idx(idx) {}

	public:
		const size_t idx;
		inline auto operator ->() const { return target[idx].get(); }
	};

	Handle<FunctionBuilder> fun(std::optional<Type> ret, std::vector<Type> args);
	Handle<ClassBuilder> type(std::optional<Handle<ClassBuilder>> base = {});
	prog::Program operator()();
};

} // namespace comp

#endif /* PROGRAMBUILDER_H_ */
