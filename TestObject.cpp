#include "1test/Test.h"

#include "Object.h"

TEST_GROUP(Object)
{
	Slot slots[16];
	SlotMeta meta[16];
	Storage storage = Storage(slots, meta, sizeof(slots)/sizeof(slots[0]));
};

TEST(Object, SingleElementCrud)
{
	Object root;
	CHECK(root.isEmpty());
	CHECK(root.length() == 0);

	{
		const auto res = root.get(storage, 0);
		CHECK(res.type == Object::Value::Type::Undefined);
	}

	CHECK(root.put(storage, 0, 0) == Object::PutResult::Inserted);

	CHECK(!root.isEmpty());
	CHECK(root.length() == 1);

	{
		const auto res = root.get(storage, 0);
		CHECK(res.type == Object::Value::Type::Integer);
		CHECK(res.data.n == 0);
	}

	{
		const auto res = root.get(storage, 1);
		CHECK(res.type == Object::Value::Type::Undefined);
	}

	CHECK(root.put(storage, 0, 1) == Object::PutResult::Inserted);
//
//	CHECK(!root.isEmpty());
//	CHECK(root.length() == 1);
//
//	{
//		const auto res = root.get(0);
//		CHECK(res.type == Object::Value::Type::Integer);
//		CHECK(res.integer == 1);
//	}
//
//	{
//		const auto res = root.get(1);
//		CHECK(res.type == Object::Value::Type::Undefined);
//	}
//
//	CHECK(root.remove(storage, 0));
//
//	CHECK(root.isEmpty());
//	CHECK(root.length() == 0);
//
//	{
//		const auto res = root.get(0);
//		CHECK(res.type == Object::Value::Type::Undefined);
//	}
};
