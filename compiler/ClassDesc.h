#ifndef COMPILER_CLASSDESC_H_
#define COMPILER_CLASSDESC_H_

#include "ValueType.h"
#include <vector>
#include <memory>

namespace comp {

struct ClassDesc
{
	const std::shared_ptr<ClassDesc> base;
	std::vector<ValueType> fieldTypes;

	inline ClassDesc(std::shared_ptr<ClassDesc> base): base(base) {};

	std::vector<size_t> getRefOffs() const
	{
		std::vector<size_t> refOffs;

		for(auto i = 0u; i < fieldTypes.size(); i++)
		{
			if(!fieldTypes[i].kind == TypeKind::Value)
			{
				refOffs.push_back(i);
			}
		}

		return refOffs;
	}

	size_t size() {
		return (base ? base->size() : 0) + fieldTypes.size();
	}
};

}//namespace comp

#endif /* COMPILER_CLASSDESC_H_ */
