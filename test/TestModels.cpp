#include "1test/Test.h"

#include "Rans.h"
#include "UnityModel.h"
#include "BinaryModel.h"
#include "FieldWidthModel.h"

TEST_GROUP(RansModel) {};

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

TEST(RansModel, Unity)
{
	for(auto n = 1u; n < 256; n++)
	{
		const auto v = checkModel(UnityModel(n));
		CHECK(n - 1 <= v && v <= n);
	}
}

TEST(RansModel, Binary)
{
	CHECK(checkModel(BinaryModel{}) == 1);
}

TEST(RansModel, FieldWidth)
{
	CHECK(checkModel(FieldWidthModel{}) == 30);
}
