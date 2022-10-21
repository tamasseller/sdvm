#include "1test/Test.h"

#include "Rans.h"

#include "UnityModel.h"
#include "BinaryModel.h"
#include "FieldWidthModel.h"

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

struct ModelInterface
{
	virtual rans::Range getRange(uint8_t d) const = 0;
	virtual rans::Range identify(uint16_t low, uint8_t& d) const = 0;
	inline virtual ~ModelInterface() = default;
};

template<class Model>
auto wrapModel(const Model& m)
{
	struct W: ModelInterface
	{
		Model m;

		inline W(const Model& m): m(m) {};
		inline virtual ~W() = default;

		virtual rans::Range getRange(uint8_t d) const {
			return m.getRange(d);
		}

		virtual rans::Range identify(uint16_t low, uint8_t& d) const {
			return m.identify(low, d);
		}
	};

	return std::static_pointer_cast<ModelInterface>(std::make_shared<W>(m));
}

TEST(Rans, MultiModelCoding)
{
	const std::vector<std::pair<std::shared_ptr<ModelInterface>, int>> models =
	{
		{wrapModel(BinaryModel{}), 1},
		{wrapModel(FieldWidthModel{}), 29},
		{wrapModel(UnityModel(12)), 11},
		{wrapModel(UnityModel(234)), 233},
	};

    std::pair<int, int> testData[128 * 1024];
    std::uniform_int_distribution<uint8_t> modelSelector(0, models.size() - 1);

    std::mt19937 mt(1337);
	for(auto& d: testData)
	{
		const auto idx = modelSelector(mt);
		std::uniform_int_distribution<uint8_t> valueSelector(0, models[idx].second);
		d = {idx, valueSelector(mt)};
	}

	VectorWriter w;
	rans::Encoder encoder;

	for(int i = sizeof(testData) / sizeof(testData[0]) - 1; 0 <= i; i--)
	{
		const auto &d = testData[i];
		encoder.put(w, *(models[d.first].first), d.second);
	}

	encoder.flush(w);

	auto result = w.done();
	ArrayReader r(result.first.get());
	rans::Decoder decoder;
	decoder.init(r);

	for(auto& d: testData)
	{
		CHECK(d.second == decoder.get<uint8_t>(r, *(models[d.first].first)));
	}
}
