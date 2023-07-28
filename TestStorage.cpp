#include <ObjectType.h>
#include "1test/Test.h"

#include "Storage.h"

TEST_GROUP(Storage)
{
	const ObjectType singleRef{nullptr, 1, {0}};

	Storage uut;
};

TEST(Storage, Sanity)
{
	auto a = uut.create(&singleRef);
	CHECK(uut.read(a, 0).reference == Storage::null);
	CHECK(0 == uut.gc(a));

	auto b = uut.create(&singleRef);
	CHECK(uut.read(b, 0).reference == Storage::null);
	CHECK(1 == uut.gc(a));

	auto c = uut.create(&singleRef);
	CHECK(uut.read(a, 0).reference == Storage::null);
	uut.write(a, 0, c);
	CHECK(uut.read(a, 0).reference == c);

	CHECK(0 == uut.gc(a));
}

TEST(Storage, Loop)
{
	auto a = uut.create(&singleRef);
	auto b = uut.create(&singleRef);
	uut.write(a, 0, b);
	auto c = uut.create(&singleRef);
	uut.write(b, 0, c);
	uut.write(c, 0, a);

	CHECK(0 == uut.gc(a));
	CHECK(0 == uut.gc(b));
	CHECK(0 == uut.gc(c));

	auto d = uut.create(&singleRef);
	CHECK(3 == uut.gc(d));
}
