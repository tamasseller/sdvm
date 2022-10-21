#ifndef CODEC_ARRAYREADER_H_
#define CODEC_ARRAYREADER_H_

#include <stdint.h>

struct ArrayReader
{
	const uint8_t* p;

	inline ArrayReader(decltype(p) data): p(data) {}

	inline uint8_t operator ()()
	{
		return *p++;
	}

	inline auto readState() const {
		return p;
	}

	inline void writeState(decltype(p) p) {
		this->p = p;
	}
};

#endif /* CODEC_ARRAYREADER_H_ */
