#ifndef STORAGE_H_
#define STORAGE_H_

#include "assert.h" // intentional single quote

#include <stddef.h>
#include <stdint.h>

class SlotMeta
{
	/*
	 * Free:  111111111
	 * func:  110- ---m    <- TBD
	 * typed: 10-- ---m    <- TBD
	 * JS:    0vvv vaam
	 *
	 *  v: valueType, k: keyType, a: avlState, m: gcMark
	 */

	static constexpr uint8_t notJsMask     = 0x80;
	static constexpr uint8_t valueTypeMask = 0x71;
	static constexpr uint8_t avlStateMask  = 0x06;
	static constexpr uint8_t gcMarkMask    = 0x01;

	static constexpr uint8_t notJsShift     = 7;
	static constexpr uint8_t valueTypeShift = 3;
	static constexpr uint8_t keyTypeShift   = 3;
	static constexpr uint8_t avlStateShift  = 1;

	uint8_t data = 0xff;

public:
	enum class ValueType: uint8_t
	{
		Integer,
		Fractional,
		Boolean,
		Undefined,
		Null,
		Array,
		Object,
		Function,
		String,
		TypedArray
	};

	inline constexpr SlotMeta() = default;
	static inline constexpr auto free() { return SlotMeta(); }
	inline constexpr SlotMeta(ValueType value, bool mark): data(
		((uint8_t)value << valueTypeShift) |
		((uint8_t)mark)
	){}

	inline bool isJs() const {
		return !(data & notJsMask);
	}

	inline bool isFree() const {
		return data == 0xff;
	}

	inline ValueType getValueType() const
	{
		assert(isJs());
		return (ValueType)(data >> valueTypeShift);
	}


};

union Slot
{
	struct Js
	{
		using Key = int32_t;

		union Data {
			int n;
			float f;
			bool b;
			Slot* ref;
		};

		Key key;
		Slot *left, *right;
		Data data;
	} js;

	struct Function
	{
		void* code;
		Slot* closure;
	} fn;

	struct TypedArray
	{
		uint8_t raw[16];
	} ta;

	struct Free
	{
		Slot* next;
	} free;
};

static_assert(sizeof(Slot) == 16);

class Storage
{
	Slot *storage, *firstFree;
	SlotMeta *metas;

public:
	Storage(Slot* storage, SlotMeta* meta, size_t count): storage(storage), firstFree(storage), metas(meta)
	{
		auto current = storage;

		for(auto i = 0u; i < count - 1; i++)
		{
			storage[i].free.next = storage + i + 1;
			meta[i] = SlotMeta::free();
		}

		storage[count -1].free.next = nullptr;
	}

	class SpaceStream
	{
		friend Storage;
		Slot* head = nullptr;

		SpaceStream(Slot* head): head(head) {}

	public:
		inline SpaceStream() = default;
		inline SpaceStream(const SpaceStream&) = delete;
		inline SpaceStream(SpaceStream&& o): head(o.head) { o.head = nullptr; }
		inline ~SpaceStream() { assert(head == nullptr); }
		inline bool operator ==(const nullptr_t&) { return head == nullptr; }
		inline bool operator !=(const nullptr_t&) { return !(*this == nullptr); }
		inline operator bool() { return *this != nullptr; }

		inline Slot* consume()
		{
			auto ret = head;
			head = head->free.next;
			return ret;
		}
	};

	SpaceStream acquire(size_t n);

	void release(Slot* s);

	inline SlotMeta &meta(Slot* slot) {
		return metas[slot - storage];
	}
};

#endif /* STORAGE_H_ */
