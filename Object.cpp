#include "Object.h"

static inline void populate(Storage &storage, Slot* slot, const Object::Key &key, const Object::Value &value)
{
	storage.meta(slot) = SlotMeta(value.type, false /* TBD */);
    slot->js.data = value.data;
    slot->js.key = key.data;
	slot->js.left = slot->js.right = nullptr;
}

static inline Object::Value load(Storage &storage, Slot* slot) {
	return Object::Value(storage.meta(slot).getValueType(), slot->js.data);
}

int slotKeyCompare(Slot::Js::Key a, Slot::Js::Key b)
{
	if(a & 1)
	{
		if(b & 1)
		{
			return a - b;
		}

		return -1;
	}
	else
	{
		if(b & 1)
		{
			return 1;
		}

		const auto slotA = (Slot*)a;
		const auto slotB = (Slot*)b;
		// TODO compare strings

		return 0;
	}
}

struct InsertionPoint
{
	Slot* parent;
	Slot** branch;
};

InsertionPoint findInsertionPoint(Slot** at, Object::Key key)
{
	InsertionPoint ret;
	return ret;
}

Object::PutResult Object::put(Storage &storage, Key key, Value value)
{
	if(root)
	{
		auto ip = findInsertionPoint(&root, key);

		if(ip.)

		return PutResult::Failed; // TODO
	}
	else
	{
		if(auto space = storage.acquire(1))
		{
			root = space.consume();
			populate(storage, root, key, value);
			return PutResult::Inserted;
		}

		return PutResult::Failed;
	}
}

static inline Slot* findSlot(Slot* slot, Slot::Js::Key key)
{
	if(!slot)
	{
		return nullptr;
	}

	const auto cmp = slotKeyCompare(slot->js.key, key);
	if(key == slot->js.key)
	{
		return slot;
	}
	else if(key < slot->js.key)
	{
		return findSlot(slot->js.left, key);
	}
	else
	{
		return findSlot(slot->js.right, key);
	}
}

Object::Value Object::get(Storage &storage, Key key) const
{
	if(auto result = findSlot(root, key.data))
	{
		return load(storage, result);
	}

	return Value(SlotMeta::ValueType::Undefined);
}

bool Object::remove(Storage &storage, Key key)
{
	return false;
}

static inline int countSlots(Slot* slot) {
	return slot ? countSlots(slot->js.left) + countSlots(slot->js.right) + 1 : 0;
}

int Object::length() const
{
	return countSlots(root);
}
