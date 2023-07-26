#ifndef STORAGE_H_
#define STORAGE_H_

#include "assert.h" // intentional single quote

#include <stddef.h>
#include <stdint.h>
#include <memory>
#include <map>

class Type;

struct Storage
{
	typedef uint32_t Ref;

	static constexpr Ref null = 0;

	Ref create(const Type* type);
	size_t gc(Ref root);

	union Value
	{
		Ref reference;
		int integer;
		float floating;
		bool logical;
		void* buffer;

		constexpr inline Value(): reference(null) {}
		constexpr inline Value(Ref reference): reference(reference) {}
		constexpr inline Value(int integer): integer(integer) {}
		constexpr inline Value(float floating): floating(floating) {}
		constexpr inline Value(bool logical): logical(logical) {}
		constexpr inline Value(void* buffer): buffer(buffer) {}
	};

	const Type* getType(Ref ref) const;
	Value read(Ref ref, size_t index) const;
	void write(Ref ref, size_t index, Value value) const;

private:
	void markWorker(Ref ref, bool mark);

	struct Record {
		bool mark;
		const Type* type;
		std::unique_ptr<Value[]> data;
	};

	uint32_t lastRef = 1;
	bool mark = false;
	std::map<Ref, Record> records;
};

#endif /* STORAGE_H_ */
