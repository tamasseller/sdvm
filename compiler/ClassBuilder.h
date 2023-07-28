#ifndef COMPILER_CLASSBUILDER_H_
#define COMPILER_CLASSBUILDER_H_

#include "object/Type.h"

#include <optional>

#include <cstdint>

namespace comp {

class ClassBuilder
{
	friend class ProgramBuilder;

	std::optional<uint32_t> baseIdx;

	inline obj::Type operator()() {
		return obj::Type::empty;
	}

public:
	inline ClassBuilder(std::optional<uint32_t> baseIdx): baseIdx(baseIdx) {}

};

} //namespace comp

#endif /* COMPILER_CLASSBUILDER_H_ */
