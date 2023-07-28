#ifndef STORAGE_H_
#define STORAGE_H_

#include "Value.h"

#include "assert.h" // intentional single quote

#include <stddef.h>
#include <stdint.h>
#include <memory>
#include <map>

namespace obj {

class Type;

struct Storage
{
	Reference create(const Type* type);
	size_t gc(Reference root);

	const Type* getType(Reference ref) const;
	Value read(Reference, size_t index) const;
	void write(Reference ref, size_t index, Value value) const;

private:
	void markWorker(Reference ref, bool mark);

	struct Record {
		bool mark;
		const Type* type;
		std::unique_ptr<Value[]> data;
	};

	uint32_t lastRef = 1;
	bool mark = false;
	std::map<Reference, Record> records;
};

} //namespace obj

#endif /* STORAGE_H_ */
