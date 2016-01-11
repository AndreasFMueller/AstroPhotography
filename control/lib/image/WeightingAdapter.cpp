/*
 * WeightingAdapter.cpp
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroAdapter.h>

namespace astro {
namespace adapter {

WeightingAdapter::WeightingAdapter(const ConstImageAdapter<double>& image,
	const ImagePoint& center, double hvr)
	: ConstImageAdapter<double>(image.getSize()), _image(image),
	  _hvr(hvr), _center(center) {
}

WeightingAdapter::WeightingAdapter(const ConstImageAdapter<double>& image,
	double hvr)
	: ConstImageAdapter<double>(image.getSize()), _image(image),
	  _hvr(hvr) {
	_center = image.getSize().center();
}

WeightingAdapter::WeightingAdapter(const ConstImageAdapter<double>& image,
	const ImageRectangle& rectangle)
	: ConstImageAdapter<double>(image.getSize()), _image(image) {
	_hvr = ceil(sqrt(getSize().width() * getSize().height() / 4.));
	_center = rectangle.center();
}

double  WeightingAdapter::pixel(int x, int y) const {
	double  r = hypot(x - _center.x(), y - _center.y()) / _hvr;
	return _image.pixel(x, y) / (1 + r * r);
}


} // namespace adapter
} // namespace astro
