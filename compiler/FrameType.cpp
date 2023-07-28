#include "FrameType.h"

#include "program/Frame.h"

#include "object/Storage.h"

static std::vector<size_t> frameTracer(const obj::Type* self, obj::Storage* storage, obj::Reference instance) {
	auto ret = obj::defaultTracer(self, storage, instance);

	// TODO add references on opstack

	return ret;
}

obj::Type comp::makeFrameType(size_t nextLocal, std::vector<size_t> frameRefIndices, size_t opStackDepth)
{
	return {&prog::Frame::base, nextLocal + opStackDepth, std::move(frameRefIndices), frameTracer};
}
