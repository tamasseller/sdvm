#ifndef PROGRAM_FRAME_H_
#define PROGRAM_FRAME_H_

#include "object/TypeInfo.h"

namespace prog {

struct Frame
{
	static constexpr auto previousFrameOffset = 0;
	static constexpr auto topOfStackOffset = 1;
	static constexpr auto offsetToLocals = 2;

	static constexpr auto callerStackExtra = 2;

	size_t opStackOffset;
	uint32_t frameTypeIndex;
};

} //namespace prog

#endif /* PROGRAM_FRAME_H_ */
