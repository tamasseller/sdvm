#include "1test/Test.h"

#include "Rans.h"
#include "UnityModel.h"
#include "ArrayReader.h"
#include "VectorWriter.h"

#include <random>

TEST_GROUP(Rans) {};

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

	auto result = w.done();
	ArrayReader r(result.first.get());
	rans::Decoder decoder;
	decoder.init(r);

	for(int i = sizeof(testData) - 1; 0 <= i; i--) {
		CHECK(testData[i] == decoder.get<uint8_t>(r, model));
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

	auto result = w.done();
	ArrayReader r(result.first.get());
	rans::Decoder decoder;
	decoder.init(r);

	for(int i = sizeof(testData) - 1; 0 <= i; i--) {
		CHECK(testData[i] == decoder.get<uint8_t>(r, model));
	}
}
