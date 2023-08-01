#include "TypeInfo.h"
#include "program/Program.h"

#include <algorithm>

using namespace obj;

size_t TypeInfo::getLength(const prog::Program& program) const {
	return (baseIdx ? program.types[baseIdx].getLength(program) : 0) + length;
}

std::pair<std::size_t, std::vector<size_t>> TypeInfo::traceWorker(const prog::Program& program, class Storage* storage, Reference instance) const
{
	auto base = baseIdx ? program.types[baseIdx].traceWorker(program, storage, instance) : std::make_pair(size_t{0}, std::vector<size_t>{});

	std::transform(refOffs.cbegin(), refOffs.cend(), std::back_inserter(base.second), [&base](auto o){ return o + base.first;});

	return {base.first + length, std::move(base.second)};
}
