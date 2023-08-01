#ifndef OBJECTTYPE_H_
#define OBJECTTYPE_H_

#include "Reference.h"

#include <vector>

namespace prog {
	class Program;
}

namespace obj {

struct TypeInfo
{
	static const obj::TypeInfo empty;

	size_t baseIdx;
	size_t length;
	std::vector<size_t> refOffs;
	bool isFrame;

	inline TypeInfo(size_t baseIdx, size_t length, std::vector<size_t> refOffs, bool isFrame = false):
		baseIdx(baseIdx), length(length), refOffs(std::move(refOffs)), isFrame(isFrame) {}

	size_t getLength(const prog::Program& program) const;

	std::pair<std::size_t, std::vector<size_t>> traceWorker(const prog::Program& program, class Storage* storage, Reference instance) const;

	inline std::vector<size_t> tracer(const prog::Program& program, class Storage* storage, Reference instance) const {
		return traceWorker(program, storage, instance).second;
	}
};

inline const obj::TypeInfo TypeInfo::empty = { .baseIdx = 0, .length = 0, .refOffs = {} };

} // namespace obj

#endif /* OBJECTTYPE_H_ */
