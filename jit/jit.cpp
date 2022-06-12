#include <cstdint>

register uint16_t** fnTabPtr asm("r12");

uint16_t* fnTab[16];

static inline uint32_t &uart()
{
	return *((uint32_t*)0x4000C000);
}

extern uint16_t jitEntry;

extern "C" void entry()
{
	fnTabPtr = fnTab;

	for(auto& f: fnTab)
	{
		f = &jitEntry;
	}

	((void(*)(uint16_t**))fnTab[0])(fnTab + 0);
}

uint16_t* buffer[1024], **ptr = buffer;

const uint16_t f, g;

static constexpr const uint16_t* const sources[] = {&f, &g};

extern "C" void jit(uint16_t** fnTabEntry)
{
	const auto idx = fnTabEntry - fnTabPtr;

	fnTabPtr[idx] = ptr;

}
