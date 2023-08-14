#ifndef OBJECT_REFERENCE_H_
#define OBJECT_REFERENCE_H_

#include <cstdint>

namespace vm {

typedef uint32_t Reference;
static constexpr Reference null = 0;

} //namespace vm

#endif /* OBJECT_REFERENCE_H_ */
