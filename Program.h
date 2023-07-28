#ifndef PROGRAM_H_
#define PROGRAM_H_

#include "Instruction.h"

#include <optional>
#include "ObjectType.h"

struct Program
{
	const ObjectType* staticType;

	struct Function
	{
		struct Frame
		{
			static constexpr auto previousFrameOffset = 0;
			static constexpr auto opStackRefChainEndOffset = 1;
			static constexpr auto topOfStackOffset = 2;
			static constexpr auto offsetToLocals = 3;

			static constexpr auto callerStackExtra = 2;

			static inline const ObjectType base =
			{
				.base = nullptr,
				.length = offsetToLocals,
				.refOffs = {0},
			};

			size_t opStackOffset;
			const ObjectType* frameType;
		} ;

		Frame frame;
		std::vector<Instruction> code;
	};

	std::vector<Function> functions;
};

#endif /* PROGRAM_H_ */
