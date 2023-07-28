#ifndef PROGRAMBUILDER_H_
#define PROGRAMBUILDER_H_

#include "FunctionBuilder.h"

#include "program/Instruction.h"
#include "program/Program.h"

#include <optional>
#include <memory>

namespace comp {

class ProgramBuilder: public prog::Program
{
	std::vector<std::unique_ptr<obj::Type>> types;
	static inline const obj::Type emptyType = { .base = nullptr, .length = 0, .refOffs = {} };

public:
	inline ProgramBuilder() {
		staticType = &emptyType;
	}

	template<class C>
	inline void fun(std::optional<Type> ret, std::vector<Type> args, C&& c) {
		auto fb = FunctionBuilder(ret, args);
		c(fb);
		functions.push_back(fb(types));
	}
};

} // namespace comp

#endif /* PROGRAMBUILDER_H_ */
