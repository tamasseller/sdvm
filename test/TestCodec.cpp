#include "1test/Test.h"

#include "Encoder.h"
#include "Decoder.h"

#include <random>

TEST_GROUP(Codec) {};

TEST(Codec, LiteralCoding)
{
	static constexpr auto n = 1234;
    std::mt19937 mt(1337);
    std::uniform_int_distribution<uint32_t> dist(0, 0xffffffff/n);

	for(int m = 0; m < 5; m++)
	{
		Encoder enc;
		const auto multiplier = dist(mt);

		for(int i = 1; i <= n; i++)
		{
			enc.writeUnsignedNonZero(multiplier * i);
		}

		const auto output = enc.done().first;
		Decoder dec(output.get());

		for(int i = 1; i <= n; i++)
		{
			CHECK(multiplier * i == dec.readUnsignedNonZero());
		}
	}
}
