#ifndef PROGRAM_H_
#define PROGRAM_H_

#include "Type.h"
#include "Instruction.h"

#include <optional>

struct Program
{
	const Type* staticType;

	struct Function
	{
		static constexpr auto previousFrameOffset = 0;
		static constexpr auto executionPointOffset = 1;

		const Type* frameType;
		std::vector<size_t> argOffsets;
		std::optional<size_t> retOffset;
		std::vector<Instruction> code;
	};

	std::vector<Function> functions;
};

#endif /* PROGRAM_H_ */
