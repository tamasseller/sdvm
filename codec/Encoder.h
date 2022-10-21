#ifndef CODEC_ENCODER_H_
#define CODEC_ENCODER_H_

#include "Rans.h"

#include "UnityModel.h"
#include "BinaryModel.h"
#include "FieldWidthModel.h"

#include "VectorWriter.h"

#include <functional>
#include <algorithm>

#include <cassert>

class Encoder
{
	std::vector<std::function<void(rans::Encoder&, VectorWriter&)>> symbols;

	template<class Model, class Data>
	inline void write(const Model& m, const Data& d) {
		symbols.push_back([m, d](rans::Encoder& e, VectorWriter& w) { e.put(w, m, d); });
	}

public:
	void writeUnsignedNonZero(uint32_t x)
	{
		assert(x);

		const auto actualWidth = 32 - clz(x);
		const uint8_t indicatedWidth = std::min(actualWidth - 1, 29);
		auto encodingWidth = indicatedWidth < 29 ? indicatedWidth : 32;

		write(FieldWidthModel{}, 29 - indicatedWidth);

		for(int i = 0; i < encodingWidth; i++)
		{
			write(BinaryModel{}, uint8_t((x & (1 << (encodingWidth - i - 1))) ? 1 : 0));
		}
	}

	auto done()
	{
		VectorWriter w;
		rans::Encoder e;

		for(auto it = symbols.rbegin(); it != symbols.rend(); it++)
		{
			(*it)(e, w);
		}

		e.flush(w);
		return w.done();
	}
};

#endif /* CODEC_ENCODER_H_ */
