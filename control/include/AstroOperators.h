/*
 * AstroOperators.h -- operators to apply to images 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroOperators_h
#define _AstroOperators_h

#include <AstroImage.h>
#include <limits>
#include <AstroDebug.h>

namespace astro {
namespace image {
namespace operators {

/**
 * \brief Image operators
 * 
 * These operators operate an an image change the image in place.
 */
template<typename T>
class ImageOperator {
public:
	virtual void	operator()(Image<T>& image) = 0;
};

template<typename T>
class FlipOperator : public ImageOperator<T> {
public:
	FlipOperator() { }
	virtual void	operator()(astro::image::Image<T>& image) {
		for (unsigned int line = 0; (line << 1) < image.size.height;
			line++) {
			T	*p = &image.pixels[line * image.size.width];
			int	lastline = image.size.height - line - 1;
			T	*q = &image.pixels[lastline * image.size.width];
			for (unsigned int i = 0; i < image.size.width; i++) {
				T	v = p[i];
				p[i] = q[i];
				q[i] = v;
			}
		}
	}
};

template<typename T>
class LimitOperator : public ImageOperator<T> {
	T	lower;
	T	upper;
public:
	LimitOperator(const T& _lower, const T& _upper)
		: lower(_lower), upper(_upper) {
	}
	virtual void	operator()(astro::image::Image<T>& image) {
		for (unsigned int i = 0; i < image.size.pixels; i++) {
			T	v = image.pixels[i];
			if (v != v) {
				continue;
			}
			if (v < lower) {
				image.pixels[i] = lower;
			}
			if (v > upper) {
				image.pixels[i] = upper;
			}
		}
	}
};

template<typename T>
class ScaleOperator : public ImageOperator<T> {
	T	lower;
	T	delta;
public:
	ScaleOperator(const T& _lower = 0,
		const T& _upper = std::numeric_limits<T>::max())
			: lower(_lower), delta(_upper - _lower) {
	}

	virtual void	operator()(astro::image::Image<T>& image) {
		T	min = image.pixels[0];
		T	max = image.pixels[0];
		for (unsigned int i = 0; i < image.size.pixels; i++) {
			T	v = image.pixels[i];
			if (v != v) {
				continue;
			}
			if (v < min) {
				min = v;
			}
			if (v > max) {
				max = v;
			}
		}
		T	span = max - min;
		for (unsigned int i = 0; i < image.size.pixels; i++) {
			T	v = image.pixels[i];
			v = lower + (v - min) * delta / span;
			image.pixels[i] = v;
		}
	}
};

} // namespace operators
} // namespace image
} // namespace astro

#endif /* _AstroOperators_h */
