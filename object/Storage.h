#ifndef STORAGE_H_
#define STORAGE_H_

#include "Value.h"

#include "program/Program.h"

#include "assert.h" // intentional single quote

#include <stddef.h>
#include <stdint.h>
#include <memory>
#include <map>

namespace obj {

class TypeInfo;

struct Storage
{
	Reference create(const prog::Program& program, uint32_t idx);
	size_t gc(const prog::Program& program, Reference root);

	const TypeInfo& getType(const prog::Program& program, Reference ref) const;
	Value read(const prog::Program& program, Reference, size_t index) const;
	void write(const prog::Program& program, Reference ref, size_t index, Value value) const;

private:
	void markWorker(const prog::Program& program, Reference ref, bool mark);

	struct Record {
		bool mark;
		uint32_t typeIdx;
		std::unique_ptr<Value[]> data;
	};

	uint32_t lastRef = 1;
	bool mark = false;
	std::map<Reference, Record> records;
};

} //namespace obj

#endif /* STORAGE_H_ */
