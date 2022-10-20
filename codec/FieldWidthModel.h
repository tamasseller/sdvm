#ifndef CODEC_FIELDWIDTHMODEL_H_
#define CODEC_FIELDWIDTHMODEL_H_

#include "Rans.h"

#include "platform/Clz.h"

struct FieldWidthModel
{
	static inline constexpr uint32_t intExp(uint8_t d)
	{
		const auto l = d & 1;
		const auto e = d >> 1;
		return ((l + 2) << e) - 2;
	}

	inline rans::Range getRange(uint8_t d) const
	{
		const auto a = intExp(d);
		const auto b = intExp(d + 1);

		return rans::Range
		{
			.start = uint32_t(a),
			.width = uint32_t(b - a)
		};
	}

	inline rans::Range identify(uint16_t low, uint8_t& d) const
	{
		const auto o = low + 2;
		const auto e = 30 - clz(o) & 31;
		const auto l = (o >> e) - 2;
		d = (e << 1) + l;

		return getRange(d);
	}
};

#endif /* CODEC_FIELDWIDTHMODEL_H_ */
