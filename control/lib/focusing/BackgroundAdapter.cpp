/*
 * BackgroundAdapter.cpp -- background adapter implementation
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "BackgroundAdapter.h"

namespace astro {
namespace focusing {

BackgroundAdapter::BackgroundAdapter(const ConstImageAdapter<float>& image,
		float l)
	: ConstImageAdapter<float>(image.getSize()), _image(image), _limit(l) {
}

float	BackgroundAdapter::pixel(int x, int y) const {
	float	v = _image.pixel(x,y);
	return (v < _limit) ? v : _limit;
}

} // namespace focusing
} // namespace astro
