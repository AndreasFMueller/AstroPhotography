/*
 * AstroMask.h -- filters to apply to images 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroMask_h
#define _AstroMask_h

#include <AstroImage.h>
#include <limits>
#include <AstroDebug.h>

namespace astro {
namespace image {

/**
 * \brief Masking functions
 *
 * The mask class needs a method to find out whether a given pixel
 *
 */
class MaskingFunction {
public:
	virtual double	operator()(size_t x, size_t y) const = 0;
};

/**
 * \brief Base class for all Masks that use a Hanning function
 */
class HanningMaskingFunction : public MaskingFunction {
protected:
	double	hanningradius;
	double	hanningfunction(double x) const;
public:
	HanningMaskingFunction(double hanningradius);
};

/**
 * \brief Rectangular Hanning Window
 */
class RectangleFunction : public HanningMaskingFunction {
	ImageRectangle	rectangle;
	ImageRectangle	innerrectangle;
	double	xmargin;
	double	ymargin;
public:
	RectangleFunction(const ImageRectangle& rectangle,
		double hanningradius = 0);
	virtual double	operator()(size_t x, size_t y) const;
}; 

/**
 * \brief Circular Hanning Window
 */
class CircleFunction : public HanningMaskingFunction {
protected:
	ImagePoint	center;
	double	radius;
public:
	CircleFunction(const ImagePoint& center, double radius,
		double hanningradius = 0);
	virtual double	operator()(size_t x, size_t y) const;
};

} // namespace image
} // namespace astro

#endif /* _AstroMask_h */
