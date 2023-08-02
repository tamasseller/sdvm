#ifndef COMPILER_STATEMENT_H_
#define COMPILER_STATEMENT_H_

#include "RValue.h"

#include <functional>

namespace comp {

struct Statement
{
	static inline std::function<void(std::vector<Line>&)> ret() {
		return [](auto& cw){cw.push_back(Line::ret(false));};
	}

	static inline std::function<void(std::vector<Line>&)> ret(const RValue& val)
	{
		return [val](auto& cw) {
			val.data->generate(cw);
			return [](auto& cw){cw.push_back(Line::ret(true));};
		};
	}

	/*	RValue create(Handle<Class> t);
		void setLocal(const Class::FieldHandle &l, const RValue& b);
	*/
};

}

#endif /* COMPILER_STATEMENT_H_ */
