#ifndef COMPILER_FUNCTIONDESC_H_
#define COMPILER_FUNCTIONDESC_H_

#include "ValueType.h"
#include "Class.h"

#include <optional>
#include <algorithm>

namespace comp {

class Line;

struct FunctionDesc
{
	std::optional<ValueType> retType;
	std::vector<Field> argFields;
	std::vector<Line> code;

	Class locals;

	inline FunctionDesc(std::optional<ValueType> ret, std::vector<ValueType> args):
			retType(ret), locals(Class::make())
	{
		std::transform(args.cbegin(), args.cend(), std::back_inserter(argFields), [this](const auto& vt){return locals.addField(vt);});
	}
};

}  // namespace comp

#endif /* COMPILER_FUNCTIONDESC_H_ */
