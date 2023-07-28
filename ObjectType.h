#ifndef OBJECTTYPE_H_
#define OBJECTTYPE_H_

#include <vector>

#include "Storage.h"

struct ObjectType
{
	const ObjectType* base;
	size_t length;
	std::vector<size_t> refOffs;

	inline ObjectType(const ObjectType* base, size_t length, std::vector<size_t> refOffs): base(base), length(length), refOffs(std::move(refOffs)) {}

	inline virtual ~ObjectType() = default;

	size_t getLength() const {
		return (base ? base->getLength() : 0) + length;
	}

	inline virtual std::vector<size_t> referenceOffsets(Storage* storage, Storage::Ref instance) const
	{
		auto ret = base ? base->referenceOffsets(storage, instance) : std::vector<size_t>{};

		ret.insert(ret.end(), refOffs.begin(), refOffs.end());

		return refOffs;
	}
};

#endif /* OBJECTTYPE_H_ */
