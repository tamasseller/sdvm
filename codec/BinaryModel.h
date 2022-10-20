#ifndef CODEC_BINARYMODEL_H_
#define CODEC_BINARYMODEL_H_

#include "Rans.h"

struct BinaryModel
{
	inline rans::Range getRange(uint8_t d) const
	{
		return rans::Range
		{
			.start = uint32_t(d << 15),
			.width = 0x8000
		};
	}

	inline rans::Range identify(uint16_t low, uint8_t& d) const
	{
		d = low >> 15;
		return getRange(d);
	}
};




#endif /* CODEC_BINARYMODEL_H_ */
