#ifndef OBJECTTYPE_H_
#define OBJECTTYPE_H_

#include <stddef.h>

namespace prog {

struct TypeInfo
{
	static const TypeInfo empty;

	size_t baseIdx;
	size_t nReferences, nScalars;

	inline TypeInfo(size_t baseIdx, size_t nReferences, size_t nScalars):
		baseIdx(baseIdx), nReferences(nReferences), nScalars(nScalars) {}

	inline bool operator==(const TypeInfo& o) const {
		return baseIdx == o.baseIdx && nScalars == o.nScalars && nReferences == o.nReferences;
	}
};

inline const TypeInfo TypeInfo::empty = { .baseIdx = 0, .nReferences = 0, .nScalars = 0 };

} // namespace prog

#endif /* OBJECTTYPE_H_ */
