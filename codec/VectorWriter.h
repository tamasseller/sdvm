#ifndef CODEC_VECTORWRITER_H_
#define CODEC_VECTORWRITER_H_

#include <vector>
#include <stdint.h>
#include <memory>

struct VectorWriter
{
	std::vector<uint8_t> data;

	inline void operator ()(uint8_t d)
	{
		data.push_back(d);
	}

	inline std::pair<std::unique_ptr<uint8_t[]>, size_t> done()
	{
		auto ret = std::unique_ptr<uint8_t[]>(new uint8_t[data.size()]);

		auto *p = ret.get();
		for(auto it = data.rbegin(); it != data.rend();)
		{
			*p++ = *it++;
		}

		return {std::move(ret), data.size()};
	}
};

#endif /* CODEC_VECTORWRITER_H_ */
