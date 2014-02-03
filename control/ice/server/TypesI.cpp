/*
 * TypesI.cpp -- conversion functions implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <TypesI.h>

namespace snowstar {

ImagePoint	convert(const astro::image::ImagePoint& point) {
	ImagePoint	result;
	result.x = point.x();
	result.y = point.y();
	return result;
}

astro::image::ImagePoint	convert(const ImagePoint& point) {
	return astro::image::ImagePoint(point.x, point.y);
}

ImageSize	convert(const astro::image::ImageSize& size) {
	ImageSize	result;
	result.width = size.width();
	result.height = size.height();
	return result;
}

astro::image::ImageSize	convert(const ImageSize& size) {
	return astro::image::ImageSize(size.width, size.height);
}

ImageRectangle	convert(const astro::image::ImageRectangle& rectangle) {
	ImageRectangle	result;
	result.origin = convert(rectangle.origin());
	result.size = convert(rectangle.size());
	return result;
}

astro::image::ImageRectangle	convert(const ImageRectangle& rectangle) {
	return astro::image::ImageRectangle(convert(rectangle.origin),
			convert(rectangle.size));
}

Point	convert(const astro::Point& point) {
	Point	result;
	result.x = point.x();
	result.y = point.y();
	return result;
}

astro::Point	convert(const Point& point) {
	return astro::Point(point.x, point.y);
}

} // namespace snowstar

