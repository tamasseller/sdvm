#include "Storage.h"

#include "TypeInfo.h"

using namespace obj;

Reference Storage::create(const prog::Program& program, uint32_t idx)
{
	auto ret = lastRef++;
	const auto& type = program.types[idx];
	records.emplace(std::make_pair(ret, Record{mark, idx, std::unique_ptr<Value[]>(new Value[type.getLength(program)])}));
	return ret;
}

const TypeInfo& Storage::getType(const prog::Program& program, Reference ref) const
{
	auto it = records.find(ref);
	assert(it != records.end());
	return program.types[it->second.typeIdx];
}

Value Storage::read(const prog::Program& program, Reference ref, size_t offset) const
{
	auto it = records.find(ref);
	assert(it != records.end());
	assert(offset < program.types[it->second.typeIdx].getLength(program));
	return it->second.data[offset];
}

void Storage::write(const prog::Program& program, Reference ref, size_t offset, Value value) const
{
	auto it = records.find(ref);
	assert(it != records.end());
	assert(offset < program.types[it->second.typeIdx].getLength(program));
	it->second.data[offset] = value;
}

void Storage::markWorker(const prog::Program& program, Reference ref, bool mark)
{
	auto it = records.find(ref);
	assert(it != records.end());
	Storage::Record &record = it->second;

	if(record.mark != mark)
	{
		record.mark = mark;
		const auto& t = program.types[record.typeIdx];
		const auto l = t.getLength(program);

		for(auto offset: t.tracer(program, this, ref))
		{
			assert(offset < l);

			if(const auto next = record.data[offset].reference; next != null)
			{
				markWorker(program, next, mark);
			}
		}
	}
}

size_t Storage::gc(const prog::Program& program, Reference root)
{
	markWorker(program, root, !mark);

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
