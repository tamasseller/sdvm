#ifndef COMPILER_FUNCTIONBUILDER_H_
#define COMPILER_FUNCTIONBUILDER_H_

#include "RValue.h"
#include "Statement.h"

#include "program/Function.h"

#include <optional>
#include "Class.h"
#include "FunctionDesc.h"

namespace comp {

class Line;

class Function
{
	friend class ProgramBuilder;

	std::shared_ptr<FunctionDesc> data;

	inline Function(std::shared_ptr<FunctionDesc> data): data(data) {}

public:
	static inline Function make(std::optional<ValueType> ret, std::vector<ValueType> args) {
		return std::make_shared<FunctionDesc>(ret, args);
	}

	inline RValue operator [](size_t n) const {
		assert(n < data->argFields.size());

		return RValue(
			[this, n](std::vector<Line>& cw) {
				cw.push_back(Line::readLocal(data->argFields[n]));
			},
			data->argFields[n].getType()
		);
	}

	template<class... Args>
	inline auto addLocal(Args&&... args) {
		return data->locals.addField(std::forward<Args>(args)...);
	}

	inline void operator << (std::function<void(std::vector<Line>&)> stmt) {
		stmt(data->code);
	}
};

} //namespace comp

#endif /* COMPILER_FUNCTIONBUILDER_H_ */
