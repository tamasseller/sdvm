#ifndef PROGRAMBUILDER_H_
#define PROGRAMBUILDER_H_

#include "Class.h"
#include "Function.h"

#include "program/Instruction.h"
#include "program/Program.h"

#include <optional>
#include <memory>
#include "Class.h"

namespace comp {

class ProgramBuilder
{
	Class globals;

public:
	ProgramBuilder();

	template<class... Args>
	inline auto addLocal(Args&&... args) {
		return globals.addField(std::forward<Args>(args)...);
	}

	prog::Program compile(Function entry);
};

} // namespace comp

#endif /* PROGRAMBUILDER_H_ */
