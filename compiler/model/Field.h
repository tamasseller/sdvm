#ifndef COMPILER_FIELD_H_
#define COMPILER_FIELD_H_

#include "Class.h"

#include <memory>

namespace comp {

struct Field
{
	std::shared_ptr<Class> type;
	uint32_t index = -1u;

	inline Field(std::shared_ptr<Class> type, uint32_t index): type(type), index(index) {}
	inline Field() = default;
	inline Field(const Field&) = default;

	inline auto getType() const {
		return type->fieldTypes[index];
	}
};

struct StaticField: Field {
	using Field::Field;
};

} //namespace comp

#endif /* COMPILER_FIELD_H_ */
