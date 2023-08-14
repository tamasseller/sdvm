#include "Storage.h"

using namespace vm;

Reference Storage::create(const prog::TypeInfo& typeInfo)
{
	auto ret = lastRef++;

	auto it = records.emplace(std::make_pair(ret, Record
	{
		mark,
		typeInfo,
		std::unique_ptr<Value[]>(new Value[typeInfo.nScalars]),
		std::unique_ptr<Reference[]>(new Reference[typeInfo.nReferences])
	}));


	for(auto i = 0u; i < it.first->second.typeInfo.nReferences; i++)
	{
		it.first->second.references[i] = null;
	}

	return ret;
}

const prog::TypeInfo& Storage::getType(Reference ref) const
{
	auto it = records.find(ref);
	assert(it != records.end());
	return it->second.typeInfo;
}

Value Storage::reads(Reference ref, size_t index) const
{
	auto it = records.find(ref);
	assert(it != records.end());
	assert(index < it->second.typeInfo.nScalars);
	return it->second.scalars[index];
}

Reference Storage::readr(Reference ref, size_t index) const
{
	auto it = records.find(ref);
	assert(it != records.end());
	assert(index < it->second.typeInfo.nReferences);
	return it->second.references[index];
}

void Storage::writes(Reference ref, size_t index, Value value) const
{
	auto it = records.find(ref);
	assert(it != records.end());
	assert(index < it->second.typeInfo.nScalars);
	it->second.scalars[index] = value;
}

void Storage::writer(Reference ref, size_t index, Reference value) const
{
	auto it = records.find(ref);
	assert(it != records.end());
	assert(index < it->second.typeInfo.nReferences);
	it->second.references[index] = value;
}

void Storage::markWorker(Reference ref, bool mark)
{
	auto it = records.find(ref);
	assert(it != records.end());
	Storage::Record &record = it->second;

	if(record.mark != mark)
	{
		record.mark = mark;

		for(auto i = 0u; i < record.typeInfo.nReferences; i++)
		{
			if(const auto r = record.references[i]; r != null)
			{
				markWorker(r, mark);
			}
		}
	}
}

size_t Storage::gc(Reference root)
{
	markWorker(root, !mark);

	size_t count = 0;
	for(auto it = records.begin(); it != records.end();)
	{
		if(it->second.mark == mark)
		{
			it = records.erase(it);
			count++;
		}
		else
		{
			++it;
		}
	}

	mark = !mark;
	return count;
}
