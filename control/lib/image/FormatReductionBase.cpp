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
	if (v < _min) { return (unsigned char)0; }
	if (v > _max) { return (unsigned char)255; }
	unsigned char	w = 255 * (v - _min) / (_max - _min);
	return w;
}

} // namespace image
} // namespace astro
