#ifndef COMPILER_CLASSDESC_H_
#define COMPILER_CLASSDESC_H_

#include "ValueType.h"

#include <vector>
#include <memory>

namespace comp {

struct Class
{
	const std::shared_ptr<Class> base;
	std::vector<ValueType> fieldTypes;
	std::vector<ValueType> staticTypes;

	inline Class(std::shared_ptr<Class> base): base(base) {};
};

}//namespace comp

#endif /* COMPILER_CLASSDESC_H_ */
