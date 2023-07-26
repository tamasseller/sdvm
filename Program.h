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
		struct Frame
		{
			static constexpr auto previousFrameOffset = 0;
			static constexpr auto opStackRefChainEndOffset = 1;
			static constexpr auto topOfStackOffset = 2;
			static constexpr auto offsetToLocals = 3;

			static constexpr auto callerStackExtra = 2;

			static inline const Type base =
			{
				.base = nullptr,
				.length = offsetToLocals,
				.refOffs = {0},
			};

			size_t opStackOffset;
			const Type* frameType;
		} ;

		Frame frame;
		std::vector<Instruction> code;
	};

	std::vector<Function> functions;
};

#endif /* PROGRAM_H_ */
