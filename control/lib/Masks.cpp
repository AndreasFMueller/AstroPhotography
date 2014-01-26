/*
 * Masks.cpp -- masks 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroFilter.h>
#include <cmath>

using namespace astro::image;

namespace astro {
namespace image {

HanningMaskingFunction::HanningMaskingFunction(double _hanningradius)
	: hanningradius(_hanningradius) {
}

/**
 * \brief Hanning windowing function 
 */
double	HanningMaskingFunction::hanningfunction(double x) const {
	double	y = cos(x * M_PI / 2.);
	return y * y;
}

/**
 * \brief Construct a rectangle function
 *
 * \param _rectangle
 * \param hanningradius	  part of the rectangle that is to be used for the
 *                        Hanning windowing function. Step function if zero.
 */
RectangleFunction::RectangleFunction(const ImageRectangle& _rectangle,
	double hanningradius)
	: HanningMaskingFunction(hanningradius), rectangle(_rectangle) {
	xmargin = hanningradius * rectangle.size().width() / 2;
	ymargin = hanningradius * rectangle.size().height() / 2;
	innerrectangle.setOrigin(ImagePoint(rectangle.origin()
					+ ImagePoint(xmargin, ymargin)));
	innerrectangle.setSize(ImageSize(rectangle.size().width() - 2 * xmargin,
		rectangle.size().height() - 2 * ymargin));
}

/**
 * \brief Masking function for a Hanning window
 *
 * \param x	x coordinate of point
 * \param y	y coordinate of point
 */
double	RectangleFunction::operator()(size_t x, size_t y) const {
	ImagePoint	point(x, y);
	if (!rectangle.contains(point)) {
		return 0;
	}
	if (innerrectangle.contains(point)) {
		return 1;
	}
	double	hx = 0;
	if ((rectangle.origin().x() <= x) && (x <= innerrectangle.origin().x())) {
		hx = (innerrectangle.origin().x() - x)/xmargin;
	}
	if (((innerrectangle.origin().x() + innerrectangle.size().width()) <= x) &&
		(x <= (rectangle.origin().x() + rectangle.size().width()))) {
		hx = (x - innerrectangle.origin().x() - innerrectangle.size().width())/xmargin;
	}
	double	hy = 0;
	if ((rectangle.origin().y() <= y) && (y <= innerrectangle.origin().y())) {
		hy = (y - innerrectangle.origin().y())/ymargin;
	}
	if (((innerrectangle.origin().y() + innerrectangle.size().height()) <= y) &&
		(y <= (rectangle.origin().y() + rectangle.size().height()))) {
		hx = (x - innerrectangle.origin().y() - innerrectangle.size().height())/ymargin;
	}
	return hanningfunction(hx) * hanningfunction(hy);
}

/**
 * \brief Construct a Circle masking function
 *
 * \param _center 	center of the circle
 * \param _radius	radius of the circle, in pixels
 * \param hanningradius part of the circle that should be used for the Hanning
 *                      window function. Step function if zero.
 */
CircleFunction::CircleFunction(const ImagePoint& _center, double _radius,
	double hanningradius) : HanningMaskingFunction(hanningradius),
		center(_center), radius(_radius) {
}

/**
 * \brief Masking function for a circular Hanning window
 */
double	CircleFunction::operator()(size_t x, size_t y) const {
	double	l = hypot((int)x - (int)center.x(), (int)y - (int)center.y()) / radius;
	if (l >= 1) {
		return 0;
	}
	if (l <= (1 - hanningradius)) {
		return 1;
	}
	if (hanningradius > 0) {
		return hanningfunction((l - 1 + hanningradius) / hanningradius);
	}
	return 0;
}

} // namespace image
} // namespace astro
