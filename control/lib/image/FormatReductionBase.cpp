/*
 * FormatReductionBase.cpp
 *
 * (c) 2018 prof Dr Andreas Müller, Hochschule Rapeprswil
 */
#include <AstroImage.h>

namespace astro {
namespace image {

FormatReductionBase::FormatReductionBase(double min, double max)
	: _min(min), _max(max) {
}

unsigned char	FormatReductionBase::clamp(double v) const {
	double	w = trunc(255 * (v - _min) / (_max - _min));
	if (w <= _min) { return (unsigned char)0; }
	if (w >= _max) { return (unsigned char)255; }
	unsigned char	result = w;
	return result;
}

} // namespace image
} // namespace astro
