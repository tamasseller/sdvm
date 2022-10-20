#ifndef CODEC_UNITYMODEL_H_
#define CODEC_UNITYMODEL_H_

#include "Rans.h"

struct UnityModel
{
	const uint32_t r;
	constexpr inline UnityModel(uint16_t n): r(65536 / n){}

	inline rans::Range getRange(uint8_t d) const
	{
		return rans::Range
		{
			.start = d * r,
			.width = r
		};
	}

	inline rans::Range identify(uint16_t low, uint8_t& d) const
	{
		d = (uint8_t)(low / r);
		return getRange(d);
	}
};

#endif /* CODEC_UNITYMODEL_H_ */
