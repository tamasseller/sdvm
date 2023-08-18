#ifndef COMPILER_FUNCTIONBUILDER_H_
#define COMPILER_FUNCTIONBUILDER_H_

#include "Helpers.h"

#include "model/Class.h"
#include "model/Function.h"
#include "model/Statement.h"

#include "program/Function.h"

#include <optional>

namespace comp {

class Line;

class FunctionBuilder
{
	friend class ProgramBuilder;

	std::shared_ptr<Function> data;
	std::shared_ptr<StatementSink> currentBlock;

	inline FunctionBuilder(std::shared_ptr<Function> data): data(data), currentBlock(std::make_shared<StatementSink>(data->body)) {}

public:
	static inline FunctionBuilder make(std::vector<ValueType> ret, std::vector<ValueType> args) {
		return std::make_shared<Function>(ret, args);
	}

//	inline RValue operator [](size_t n) const {
//		assert(n < data->argFields.size());
//
//		return RValue(
//			[this, n](std::vector<Line>& cw) {
//				cw.push_back(Line::readLocal(data->argFields[n]));
//			},
//			data->argFields[n].getType()
//		);
//	}

	template<class C>
	inline auto operator()(C&& stmt) {
		return stmt(currentBlock);
	}
};

} //namespace comp

#endif /* COMPILER_FUNCTIONBUILDER_H_ */
