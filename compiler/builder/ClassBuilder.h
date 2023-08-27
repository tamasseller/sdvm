#ifndef COMPILER_CLASS_H_
#define COMPILER_CLASS_H_

#include "compiler/ast/Class.h"
#include "compiler/ast/Field.h"
#include "compiler/ast/ValueType.h"
#include "compiler/ast/Values.h"

#include "Helpers.h"

namespace comp {

class ClassBuilder
{
	friend class FunctionBuilder;

	inline ClassBuilder(std::shared_ptr<ast::Class> data): data(data) {}

public:
	const std::shared_ptr<ast::Class> data;

	inline ClassBuilder() = default;

	static inline ClassBuilder make(ClassBuilder base = {}) {
		return {std::make_shared<ast::Class>(base.data)};
	}

	inline ast::Field addField(ast::ValueType vt)
	{
		const auto idx = data->fieldTypes.size();
		data->fieldTypes.push_back(vt);
		return {data, idx};
	}

	inline auto addField(const ClassBuilder& c) {
		return addField(ast::ValueType::reference(c.data));
	}

	inline ast::StaticField addStaticField(ast::ValueType vt)
	{
		const auto idx = data->staticTypes.size();
		data->staticTypes.push_back(vt);
		return {data, idx};
	}

	inline auto addStaticField(const ClassBuilder& c) {
		return addStaticField(ast::ValueType::reference(c.data));
	}

	inline RValWrapper operator()() {
		return {std::make_shared<ast::Create>(data)};
	}

	inline LValWrapper operator[](const ast::StaticField& sf) const {
		assert(sf.type == data);
		return {std::make_shared<ast::Global>(sf)};
	}
};

} //namespace comp

#endif /* COMPILER_CLASS_H_ */
