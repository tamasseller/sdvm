#ifndef COMPILER_FRAMETYPE_H_
#define COMPILER_FRAMETYPE_H_

#include "object/Type.h"

namespace comp {

struct FrameType: obj::Type
{
	FrameType(size_t nextLocal, std::vector<size_t> frameRefIndices, size_t opStackDepth):
		FrameType::Type(&prog::Frame::base, nextLocal + opStackDepth, std::move(frameRefIndices)) {}

	inline virtual std::vector<size_t> referenceOffsets(obj::Storage* storage, obj::Reference instance) const {
		auto ret = this->obj::Type::referenceOffsets(storage, instance);

		// TODO walk up the opstack

		return ret;
	}
};

} //namespace comp

#endif /* COMPILER_FRAMETYPE_H_ */
