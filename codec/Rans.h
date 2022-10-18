#ifndef CODEC_RANS_H_
#define CODEC_RANS_H_

#include <stdint.h>

namespace rans
{
	static constexpr uint32_t rangeScaleBits = 16;
	static constexpr uint32_t normalizedLowerBound = 1u << 24;
	static constexpr uint32_t maxMult = 1u << (32 - rangeScaleBits);
	static constexpr uint32_t mask = (1u << rangeScaleBits) - 1;

	struct Range
	{
		uint32_t start, width;
	};

	class Encoder
	{
		uint32_t x = normalizedLowerBound;

	public:
		inline Encoder() = default;

		template<class Writer, class Model, class Data>
		inline void put(Writer& w, const Model& m, const Data& d)
		{
			const Range range = m.getRange(d);

			for(; x >= maxMult * range.width; x >>= 8)
				w((uint8_t)x);

			x = ((x / range.width) << rangeScaleBits) + (x % range.width) + range.start;
		}

		template<class Writer>
		inline void flush(Writer& w)
		{
			for(auto i = 0u; i < 4; i++)
			{
				w((uint8_t)x);
				x >>= 8;
			}
		}
	};

	class Decoder
	{
		uint32_t x;

	public:
		template<class Data, class Reader, class Model>
		inline Data get(Reader& r, const Model& m)
		{
			const auto low = x & mask;

			Data ret;
			const Range range = m.identify(low, ret);

			x = range.width * (x >> rangeScaleBits) + low - range.start;

			while(x < normalizedLowerBound)
			{
				x = (x << 8) | (uint8_t)r();
			}

			return ret;
		}

		template<class Reader>
		inline void init(Reader& r)
		{
			for(auto i = 0u; i < 4; i++)
			{
				x = (x << 8) | (uint8_t)r();
			}
		}
	};
}

#endif /* CODEC_RANS_H_ */
