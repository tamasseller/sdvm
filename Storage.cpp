#include "Storage.h"
#include "Type.h"

Storage::Ref Storage::create(const Type* type)
{
	auto ret = lastRef++;
	records.emplace(std::make_pair(ret, Record{mark, type, std::unique_ptr<Value[]>(new Value[type->getLength()])}));
	return ret;
}

const Type* Storage::getType(Ref ref) const
{
	auto it = records.find(ref);
	assert(it != records.end());
	return it->second.type;
}

Storage::Value Storage::read(Ref ref, size_t offset) const
{
	auto it = records.find(ref);
	assert(it != records.end());
	assert(offset < it->second.type->getLength());
	return it->second.data[offset];
}

void Storage::write(Ref ref, size_t offset, Value value) const
{
	auto it = records.find(ref);
	assert(it != records.end());
	assert(offset < it->second.type->getLength());
	it->second.data[offset] = value;
}

void Storage::markWorker(Ref ref, bool mark)
{
	auto it = records.find(ref);
	assert(it != records.end());
	Storage::Record &record = it->second;

	if(record.mark != mark)
	{
		record.mark = mark;

		for(auto offset: record.type->referenceOffsets(this, ref))
		{
			assert(offset < record.type->getLength());

			if(const auto next = record.data[offset].reference; next != null)
			{
				markWorker(next, mark);
			}
		}
	}
}

size_t Storage::gc(Ref root)
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
