#include "Rans.h"

#include "1test/Test.h"

#include "Rans.h"

#include <vector>
#include <cassert>
#include <algorithm>
#include <random>

TEST_GROUP(Rans) {};

struct VectorWriter
{
	std::vector<uint8_t> data;

	inline void operator ()(uint8_t d)
	{
		data.push_back(d);
	}

	inline auto done()
	{
		auto ret = std::move(data);
		std::reverse(ret.begin(), ret.end());
		return ret;
	}
};

struct VectorReader
{
	const std::vector<uint8_t> data;
	decltype(data)::const_iterator it;

	inline VectorReader(decltype(data) data): data(std::move(data)), it(this->data.begin()) {}

	inline uint8_t operator ()()
	{
		assert(it != data.end());
		return *it++;
	}
};

struct UnityModel
{
	const uint32_t r;
	constexpr inline UnityModel(uint16_t n): r(65536 / n){}

	inline rans::Range getRange(uint8_t d) const
	{
		return rans::Range
		{
			.start = d * r,
			.width = r
		};
	}

	inline rans::Range identify(uint16_t low, uint8_t& d) const
	{
		d = (uint8_t)(low / r);
		return getRange(d);
	}
};

template<class Model>
uint8_t checkModel(const Model& uut)
{
	uint8_t last = 0;

	for(auto i = 0u; i < (1u << 16); i++)
	{
		uint8_t d;
		const auto dRange = uut.identify(i, d);
		CHECK(dRange.start <= i && i < dRange.start + dRange.width);

		const auto eRange = uut.getRange(d);
		CHECK(dRange.start == eRange.start && dRange.width == eRange.width);

		if(d != last)
		{
			CHECK(d == last + 1);
			const auto pRange = uut.getRange(last);
			CHECK(pRange.start + pRange.width == eRange.start);
		}

		last = d;
	}

	return last;
}

TEST(Rans, ModelSanity)
{
	for(auto n = 1u; n < 256; n++)
	{
		const auto v = checkModel(UnityModel(n));
		CHECK(n - 1 <= v && v <= n);
	}
}

TEST(Rans, SingleModelCoding)
{
    std::mt19937 mt(1337);
    std::uniform_int_distribution<uint8_t> dist(0, 122);
    uint8_t testData[1024 * 1024];

	for(auto& d: testData) {
		d = dist(mt);
	}

	VectorWriter w;
	rans::Encoder encoder;
	const auto model = UnityModel(123);

	for(const auto& d: testData) {
		encoder.put(w, model, d);
	}

	encoder.flush(w);

	VectorReader r(w.done());
	rans::Decoder decoder;
	decoder.init(r);

	for(int i = sizeof(testData) - 1; 0 <= i; i--) {
		CHECK(testData[i] == decoder.get<uint8_t>(r, model));
	}
}
