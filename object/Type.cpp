#include "Type.h"

using namespace obj;

std::vector<size_t> obj::defaultTracer(const Type* self, class Storage* storage, Reference instance)
{
	auto ret = self->base ? self->base->tracer(self, storage, instance) : std::vector<size_t>{};

	ret.insert(ret.end(), self->refOffs.cbegin(), self->refOffs.cend());

	return ret;
}
