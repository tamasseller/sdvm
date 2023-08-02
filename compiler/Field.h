#ifndef COMPILER_FIELD_H_
#define COMPILER_FIELD_H_

#include <memory>

namespace comp {

class Class;

class Field
{
	friend Class;
	inline Field(std::shared_ptr<ClassDesc> type, uint32_t index): type(type), index(index) {}

public:
	std::shared_ptr<ClassDesc> type;
	uint32_t index = -1u;

	inline Field() = default;
	inline Field(const Field&) = default;

	inline auto getType() {
		return type->fieldTypes[index];
	}
};

}//namespace comp

#endif /* COMPILER_FIELD_H_ */
