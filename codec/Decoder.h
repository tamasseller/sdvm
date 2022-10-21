#ifndef CODEC_DECODER_H_
#define CODEC_DECODER_H_

#include "Rans.h"

#include "UnityModel.h"
#include "BinaryModel.h"
#include "FieldWidthModel.h"

#include "ArrayReader.h"

class Decoder: rans::Decoder, ArrayReader
{
	friend rans::Decoder;
	friend ArrayReader;
public:
	inline Decoder(decltype(p) data): ArrayReader(data) {
		this->Decoder::init(*this);
	}

	struct Checkpoint
	{
		uint32_t decoderState;
		const uint8_t* readerState;
	};

	inline Checkpoint createCheckpoint()
	{
		return Checkpoint {
			.decoderState = this->rans::Decoder::readState(),
			.readerState = this->ArrayReader::readState()
		};
	}

	inline void writeState(const Checkpoint& cp) {
		this->rans::Decoder::writeState(cp.decoderState);
		this->ArrayReader::writeState(cp.readerState);
	}

	uint32_t readUnsignedNonZero()
	{
		const auto indicatedWidth = 29 - this->rans::Decoder::get<uint8_t>(*this, FieldWidthModel{});
		auto encodingWidth = indicatedWidth < 29 ? indicatedWidth : 32;

		uint32_t ret = 1;
		while(encodingWidth--)
		{
			const auto b = this->rans::Decoder::get<uint8_t>(*this, BinaryModel{});
			ret = (ret << 1) | b;
		}

		return ret;
	}
};

#endif /* CODEC_DECODER_H_ */
