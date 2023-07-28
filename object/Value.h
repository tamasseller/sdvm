#ifndef OBJECT_VALUE_H_
#define OBJECT_VALUE_H_

#include "Reference.h"

namespace obj {

union Value
{
	Reference reference;
	int integer;
	float floating;
	bool logical;
	void* buffer;

	constexpr inline Value(): reference(null) {}
	constexpr inline Value(Reference reference): reference(reference) {}
	constexpr inline Value(int integer): integer(integer) {}
	constexpr inline Value(float floating): floating(floating) {}
	constexpr inline Value(bool logical): logical(logical) {}
	constexpr inline Value(void* buffer): buffer(buffer) {}
};

} //namespace obj

#endif /* OBJECT_VALUE_H_ */
