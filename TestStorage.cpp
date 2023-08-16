#include "program/TypeInfo.h"
#include "vm/Storage.h"

#include "1test/Test.h"

TEST_GROUP(Storage)
{
	const prog::TypeInfo singleRef = prog::TypeInfo{0, 1, 0};
	const prog::TypeInfo listElement = prog::TypeInfo{0, 1, 1};

	vm::Storage uut;
};

TEST(Storage, Sanity)
{
	auto a = uut.create(singleRef);
	CHECK(uut.getType(a) == singleRef);
	CHECK(uut.readr(a, 0) == vm::null);
	CHECK(0 == uut.gc(a));

	auto b = uut.create(singleRef);
	CHECK(uut.readr(b, 0) == vm::null);
	CHECK(1 == uut.gc(a));

	auto c = uut.create(singleRef);
	CHECK(uut.readr(a, 0) == vm::null);
	uut.writer(a, 0, c);
	CHECK(uut.readr(a, 0) == c);

	CHECK(0 == uut.gc(a));
}

TEST(Storage, Loop)
{
	auto a = uut.create(singleRef);
	auto b = uut.create(singleRef);
	uut.writer(a, 0, b);
	auto c = uut.create(singleRef);
	uut.writer(b, 0, c);
	uut.writer(c, 0, a);

	CHECK(0 == uut.gc(a));
	CHECK(0 == uut.gc(b));
	CHECK(0 == uut.gc(c));

	auto d = uut.create(singleRef);
	CHECK(3 == uut.gc(d));
}

TEST(Storage, List)
{
	vm::Reference tail = vm::null;
	auto head = tail;

	for(int i = 0; i < 10 ; i++)
	{
		auto next = uut.create(listElement);
		uut.writes(next, 0, i);

		if(tail != vm::null)
		{
			uut.writer(tail, 0, next);
		}
		else
		{
			head = next;
		}

		tail = next;
	}

	CHECK(0 == uut.gc(head));

	auto p = head;
	for(int i = 0; i < 10 ; i++)
	{
		CHECK(p != vm::null);
		CHECK(i == uut.reads(p, 0).integer);
		p = uut.readr(p, 0);
	}

	CHECK(p == vm::null);

	uut.writer(head, 0, vm::null);

	CHECK(9 == uut.gc(head));
}
