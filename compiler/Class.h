#ifndef COMPILER_CLASS_H_
#define COMPILER_CLASS_H_

#include "ValueType.h"
#include "ClassDesc.h"
#include "Field.h"

namespace comp {

class Class
{
	friend class ProgramBuilder;

	std::shared_ptr<ClassDesc> data;

	inline Class(std::shared_ptr<ClassDesc> data): data(data) {}

public:
	inline Class() = default;

	static inline Class make(Class base = {}) {
		return std::make_shared<ClassDesc>(base.data);
	}

	inline Field addField(ValueType vt)
	{
		const auto idx = data->fieldTypes.size();
		data->fieldTypes.push_back(vt);
		return {data, idx};
	}

	inline auto addField(Class c) {
		return addField(ValueType::reference(c.data));
	}
};

} //namespace comp

#endif /* COMPILER_CLASS_H_ */
