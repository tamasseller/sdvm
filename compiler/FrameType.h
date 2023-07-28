#ifndef COMPILER_FRAMETYPE_H_
#define COMPILER_FRAMETYPE_H_

#include "object/Type.h"

namespace comp {

obj::Type makeFrameType(size_t nextLocal, std::vector<size_t> frameRefIndices, size_t opStackDepth);

} //namespace comp

#endif /* COMPILER_FRAMETYPE_H_ */
