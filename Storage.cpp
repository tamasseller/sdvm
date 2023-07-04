#include "Storage.h"

Storage::SpaceStream Storage::acquire(size_t n)
{
	auto last = firstFree;

	while(--n)
	{
		if(!(last = last->free.next)) {
			return {nullptr};
		}
	}

	auto ret = firstFree;
	firstFree = last->free.next;
	last->free.next = nullptr;
	return {ret};
}

void Storage::release(Slot* s)
{
	auto idx = s - storage;
	metas[idx] = SlotMeta::free();
	s->free.next = firstFree;
	firstFree = s;
}
