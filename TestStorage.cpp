#include "1test/Test.h"

#include "Storage.h"

TEST_GROUP(Storage)
{
	Slot slots[16];
	SlotMeta meta[16];
	Storage uut = Storage(slots, meta, sizeof(slots)/sizeof(slots[0]));
};

TEST(Storage, Sanity)
{
	for(auto i = 0; i < 100; i++)
	{
		auto ret = uut.acquire(1);
		CHECK(ret);
		CHECK(ret != nullptr);
		CHECK(!(ret == nullptr));

		auto slot = ret.consume();
		CHECK(!ret);
		CHECK(ret == nullptr);
		CHECK(!(ret != nullptr));

		CHECK(slot);

		uut.release(slot);
	}
}

TEST(Storage, DepleteAtOnce)
{
	for(auto i = 0; i < 100; i++)
	{
		static constexpr auto n = sizeof(slots)/sizeof(slots[0]);
		auto ret = uut.acquire(n);
		CHECK(ret);
		CHECK(ret != nullptr);
		CHECK(!(ret == nullptr));

		for(auto j = 0u; j < n; j++)
		{
			auto slot = ret.consume();
			CHECK(slot);
			uut.release(slot);
		}

		CHECK(!ret);
		CHECK(ret == nullptr);
		CHECK(!(ret != nullptr));
	}
}
