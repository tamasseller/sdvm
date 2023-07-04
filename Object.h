#ifndef OBJECT_H_
#define OBJECT_H_

#include "Storage.h"

class Object
{
	Slot* root = nullptr;

public:
	struct Value {
		using Type = SlotMeta::ValueType;
		Type type;
		Slot::Js::Data data;

		inline Value(SlotMeta::ValueType type): type(type) {}
		inline Value(SlotMeta::ValueType type, Slot::Js::Data data): type(type), data(data) {}
		inline Value(int i): type(SlotMeta::ValueType::Integer), data {.n = i} {}

/*		constexpr inline Value(float f): type(Type::Float), fractional(f) {}
		constexpr inline Value(bool b): type(Type::Boolean), boolean(b) {}
		constexpr inline Value(Type type, Slot* ref = nullptr): type(type), ref(ref) {}*/
	};

	struct Key {
		const int32_t data;

		static_assert(sizeof(void*) == sizeof(int32_t), "32bit only");
		static_assert(1 < alignof(Slot*));

		inline bool isInteger() const {
			return data & 1;
		}

		inline int getInteger() const {
			return data >> 1;
		}

		inline Slot *getName() const {
			return (Slot *)data;
		}

		inline constexpr Key(int integer): data((integer << 1) | 1) {}
		inline constexpr Key(Slot* string): data((int32_t)string) {}
	};

	enum class PutResult {
		Inserted, Updated, Failed
	};

	PutResult put(Storage &storage, Key key, Value value);
	Value get(Storage &storage, Key key) const;
	bool remove(Storage &storage, Key key);

	int length() const;

	inline bool isEmpty() const {
		return root == nullptr;
	}
};

#endif /* OBJECT_H_ */
