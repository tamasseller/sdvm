#ifndef TYPE_H_
#define TYPE_H_

#include <vector>

#include "Storage.h"

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

	inline virtual std::vector<size_t> referenceOffsets(Storage* storage, Storage::Ref instance) const
	{
		auto ret = base ? base->referenceOffsets(storage, instance) : std::vector<size_t>{};

		ret.insert(ret.end(), refOffs.begin(), refOffs.end());

		return refOffs;
	}
};

#endif /* TYPE_H_ */
