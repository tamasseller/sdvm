#ifndef OBJECTTYPE_H_
#define OBJECTTYPE_H_

#include "Reference.h"

#include <vector>

namespace obj {

struct Type
{
	const Type* base;
	size_t length;
	std::vector<size_t> refOffs;

	inline Type(const Type* base, size_t length, std::vector<size_t> refOffs): base(base), length(length), refOffs(std::move(refOffs)) {}

	inline virtual ~Type() = default;

	size_t getLength() const {
		return (base ? base->getLength() : 0) + length;
	}

	inline virtual std::vector<size_t> referenceOffsets(class Storage* storage, Reference instance) const
	{
		auto ret = base ? base->referenceOffsets(storage, instance) : std::vector<size_t>{};

		ret.insert(ret.end(), refOffs.begin(), refOffs.end());

		return refOffs;
	}
};

} // namespace obj

#endif /* OBJECTTYPE_H_ */
