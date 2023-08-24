#ifndef OBJECT_VALUE_H_
#define OBJECT_VALUE_H_

namespace vm {

union Value
{
	int integer;
	float floating;
	bool logical;
	void* buffer;

	constexpr inline Value(): buffer(nullptr) {}
	constexpr inline Value(int integer): integer(integer) {}
	constexpr inline Value(float floating): floating(floating) {}
	constexpr inline Value(bool logical): logical(logical) {}
	constexpr inline Value(void* buffer): buffer(buffer) {}
};

} //namespace vm

#endif /* OBJECT_VALUE_H_ */
