#ifndef PROGRAM_FRAME_H_
#define PROGRAM_FRAME_H_

#include "object/Type.h"

namespace prog {

struct Frame
{
	static constexpr auto previousFrameOffset = 0;
	static constexpr auto opStackRefChainEndOffset = 1;
	static constexpr auto topOfStackOffset = 2;
	static constexpr auto offsetToLocals = 3;

	static constexpr auto callerStackExtra = 2;

	static inline const obj::Type base =
	{
		.base = nullptr,
		.length = offsetToLocals,
		.refOffs = {0},
	};

	size_t opStackOffset;
	uint32_t frameTypeIndex;
};

} //namespace prog

#endif /* PROGRAM_FRAME_H_ */
