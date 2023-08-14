#ifndef STORAGE_H_
#define STORAGE_H_

#include "Value.h"
#include "Reference.h"

#include "program/TypeInfo.h"

#include "assert.h" // intentional single quote

#include <stddef.h>
#include <stdint.h>
#include <memory>
#include <map>

namespace vm {

struct Storage
{
	Reference create(const prog::TypeInfo &typeInfo);
	size_t gc(Reference root);

	const prog::TypeInfo& getType(Reference ref) const;

	Value reads(Reference ref, size_t index) const;
	Reference readr(Reference ref, size_t index) const;
	void writes(Reference ref, size_t index, Value value) const;
	void writer(Reference ref, size_t index, Reference value) const;

private:
	void markWorker(Reference ref, bool mark);

	struct Record
	{
		bool mark;
		const prog::TypeInfo typeInfo;
		std::unique_ptr<Value[]> scalars;
		std::unique_ptr<Reference[]> references;
	};

	uint32_t lastRef = 1;
	bool mark = false;
	std::map<Reference, Record> records;
};

} //namespace vm

#endif /* STORAGE_H_ */
