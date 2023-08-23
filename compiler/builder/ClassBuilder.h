#ifndef COMPILER_CLASS_H_
#define COMPILER_CLASS_H_

#include "compiler/model/Field.h"
#include "compiler/model/Class.h"
#include "compiler/model/Create.h"
#include "compiler/model/Global.h"
#include "compiler/model/ValueType.h"

#include "Helpers.h"

namespace comp {

class ClassBuilder
{
	friend class FunctionBuilder;

	inline ClassBuilder(std::shared_ptr<Class> data): data(data) {}

public:
	const std::shared_ptr<Class> data;

	inline ClassBuilder() = default;

	static inline ClassBuilder make(ClassBuilder base = {}) {
		return {std::make_shared<Class>(base.data)};
	}

	inline Field addField(ValueType vt)
	{
		const auto idx = data->fieldTypes.size();
		data->fieldTypes.push_back(vt);
		return {data, idx};
	}

	inline auto addField(const ClassBuilder& c) {
		return addField(ValueType::reference(c.data));
	}

	inline StaticField addStaticField(ValueType vt)
	{
		const auto idx = data->staticTypes.size();
		data->staticTypes.push_back(vt);
		return {data, idx};
	}

	inline auto addStaticField(const ClassBuilder& c) {
		return addStaticField(ValueType::reference(c.data));
	}

	inline RValWrapper operator()() {
		return {std::make_shared<Create>(data)};
	}

	inline LValWrapper operator[](const StaticField& sf) const {
		assert(sf.type == data);
		return {std::make_shared<Global>(sf)};
	}
};

} //namespace comp

#endif /* COMPILER_CLASS_H_ */
