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

/**
 * \brief Operator to verticall flip an image
 */
template<typename T>
class FlipOperator : public ImageOperator<T> {
public:
	FlipOperator() { }
	virtual void	operator()(astro::image::Image<T>& image) {
		int	h = image.size().height();
		int	w = image.size().width();
		for (int line = 0; (line << 1) < (h - 1); line++) {
			T	*p = &image.pixels[line * w];
			int	lastline = h - line - 1;
			T	*q = &image.pixels[lastline * w];
			for (int i = 0; i < w; i++) {
				T	v = p[i];
				p[i] = q[i];
				q[i] = v;
			}
		}
	}
};

template<typename T>
void	flip(Image<T>& image) {
	FlipOperator<T>()(image);
}

void	flip(ImagePtr image);

template<typename T>
class HFlipOperator : public ImageOperator<T> {
public:
	HFlipOperator() { }
	virtual void	operator()(astro::image::Image<T>& image) {
		int	h = image.size().height();
		int	w = image.size().width();
		int	w2 = w >> 1;
		for (int line = 0; line < h; line++) {
			T	*p = &image.pixels[line * w];
			for (int x = 0; x < w2; x++) {
				T	v = p[x];
				p[x] = p[w - 1 - x];
				p[w - 1 - x] = v;
			}
		}
	}
};

template<typename T>
void	hflip(Image<T>& image) {
	HFlipOperator<T>()(image);
}

void	hflip(ImagePtr image);

/**
 * \brief Operator to limit pixel values
 */
template<typename T>
class LimitOperator : public ImageOperator<T> {
	T	lower;
	T	upper;
public:
	LimitOperator(const T& _lower, const T& _upper)
		: lower(_lower), upper(_upper) {
	}
	virtual void	operator()(astro::image::Image<T>& image) {
		for (unsigned int i = 0; i < image.size().getPixels(); i++) {
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
void	limit(Image<T>& image, const T& _lower, const T& _upper) {
	LimitOperator<T>(_lower, _upper)(image);
}

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
		for (unsigned int i = 0; i < image.size().getPixels(); i++) {
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
		for (unsigned int i = 0; i < image.size().pixels; i++) {
			T	v = image.pixels[i];
			v = lower + (v - min) * delta / span;
			image.pixels[i] = v;
		}
	}
};

template<typename T>
class ColorScalingOperator : public ImageOperator<RGB<T> > {
	RGB<double>	_scale;
public:
	ColorScalingOperator(const RGB<double>& scale) : _scale(scale) {
	}
	virtual void	operator()(Image<RGB<T> >& image) {
		int	w = image.size().width();
		int	h = image.size().height();
		for (int x = 0; x < w; x++) {
			for (int y = 0; y < h; y++) {
				RGB<T>	v = image.pixel(x, y);
				image.pixel(x, y) = RGB<T>(v.R * _scale.R,
					v.G * _scale.G, v.B * _scale.B);
			}
		}
	}
};

void	colorscaling_operator(const RGB<double>& scale, ImagePtr image);

} // namespace operators
} // namespace image
} // namespace astro

#endif /* _AstroOperators_h */
