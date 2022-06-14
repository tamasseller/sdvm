#include <cstdint>
#include <cstddef>

static uint16_t* fnTab[16];

static uint16_t buffer[1024];
static uint16_t *ptr;

extern "C" void startVm(uint16_t **fnTab, size_t nFnTab);

extern "C" void entry()
{
	ptr = buffer;
	startVm(fnTab, sizeof(fnTab) / sizeof(fnTab[0]));
}

extern "C" const uint16_t f, fEnd, g, gEnd;

static constexpr const struct Func { const uint16_t* begin, *end; } sources[] =
{
	{&f, &fEnd},
	{&g, &gEnd}
};

extern "C" void jit(uint16_t** fnTabEntry)
{
	const auto idx = fnTabEntry - fnTab;

	fnTab[idx] = (uint16_t*)((uintptr_t)ptr | 1u);

	const auto beg = (uint16_t*)((uintptr_t)sources[idx].begin & ~1u);
	const auto end = (uint16_t*)((uintptr_t)sources[idx].end & ~1u);

	for(const uint16_t* src = beg; src != end;)
	{
		*ptr++ = *src++;
	}
}
