#ifndef OBJECTTYPE_H_
#define OBJECTTYPE_H_

#include "Reference.h"

#include <vector>

namespace obj {

class Type;

std::vector<size_t> defaultTracer(const Type* self, class Storage* storage, Reference instance);

struct Type
{
	static const obj::Type empty;

	const Type* base;
	size_t length;
	std::vector<size_t> refOffs;
	std::vector<size_t> (*tracer)(const Type* self, class Storage* storage, Reference instance);

	inline Type(const Type* base, size_t length, std::vector<size_t> refOffs, decltype(tracer) tracer = defaultTracer):
		base(base), length(length), refOffs(std::move(refOffs)), tracer(tracer) {}

	size_t getLength() const {
		return (base ? base->getLength() : 0) + length;
	}

};

inline const obj::Type Type::empty = { .base = nullptr, .length = 0, .refOffs = {} };


} // namespace obj

#endif /* OBJECTTYPE_H_ */
