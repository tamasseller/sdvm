#ifndef COMPILER_CLASSBUILDER_H_
#define COMPILER_CLASSBUILDER_H_

#include "Handle.h"

#include "ValueType.h"

#include "object/TypeInfo.h"

#include <optional>

#include <cstdint>

namespace comp {

class ClassBuilder
{
	friend class ProgramBuilder;

	const uint32_t ownIdx;
	const std::optional<uint32_t> baseIdx;
	std::vector<ValueType> fieldTypes;
	bool isFrame;

	obj::TypeInfo operator()();

public:
	inline ClassBuilder(uint32_t ownIdx, std::optional<uint32_t> baseIdx, bool isFrame = false): ownIdx(ownIdx), baseIdx(baseIdx), isFrame(isFrame) {}

	struct FieldHandle
	{
		const uint32_t typeIdx, fieldIdx;
		ValueType type;
	};

	inline FieldHandle addField(ValueType vt)
	{
		const auto idx = fieldTypes.size();
		fieldTypes.push_back(vt);
		return FieldHandle {
			.typeIdx = ownIdx,
			.fieldIdx = idx,
			.type = vt
		};
	}

	inline auto addField(Handle<ClassBuilder> c) {
		return addField(ValueType::reference(c.idx));
	}

	inline auto size() const {
		return fieldTypes.size();
	}
};

} //namespace comp

#endif /* COMPILER_CLASSBUILDER_H_ */
