/*
 * AstroFilter.h -- filters to apply to images 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroFilter_h
#define _AstroFilter_h

#include <AstroImage.h>
#include <limits>
#include <debug.h>

namespace astro {
namespace image {
namespace filter {

/**
 * \brief Subgrid speicification
 *
 * Some filter operations only happen on a subgrid, specified by this 
 * class
 */
class	Subgrid {
public:
	ImagePoint	origin;
	ImageSize	size;
	Subgrid() : origin(ImagePoint(0, 0)), size(ImageSize(1, 1)) { }
	Subgrid(const ImagePoint& _origin, const ImageSize& _size)
		: origin(_origin), size(_size) { }
	size_t	volume() const { return size.width * size.height; }
};

/**
 * \brief Filters that return a single value of the same type as the image.
 *
 * This type of filter cannot be used to compute values from the image
 * that don't fit in the pixel type. An example of such a value would be
 * the mean value. There is a Mean filter derived from this type but in its
 * basic form it computes the integer rounded version.
 */
template<typename T, typename S>
class PixelTypeFilter {
public:
	virtual S	filter(const astro::image::Image<T>& image,
				Subgrid subgrid = Subgrid()) = 0;
	virtual T	operator()(const astro::image::Image<T>& image,
				Subgrid subgrid = Subgrid()) = 0;
};

/**
 * \brief Filter to count NaNs
 */
template<typename T, typename S>
class CountNaNs : public PixelTypeFilter<T, S> {
public:
	CountNaNs() { }
	virtual S	filter(const astro::image::Image<T>& image,
				const Subgrid grid = Subgrid());
	virtual T	operator()(const astro::image::Image<T>& image,
				const Subgrid grid = Subgrid());
};

template<typename T, typename S>
S	CountNaNs<T, S>::filter(const astro::image::Image<T>& image,
			const Subgrid grid) {
	S	result = 0;
	for (unsigned int x = grid.origin.x; x < image.size.width;
		x += grid.size.width) {
		for (unsigned int y = grid.origin.y;
			y < image.size.height; y += grid.size.height) {
			T	v = image.pixel(x, y);
			if (v != v) {
				result += 1;
			}
		}
	}
	return result;
}

template<typename T, typename S>
T	CountNaNs<T, S>::operator()(const astro::image::Image<T>& image,
			const Subgrid grid) {
	return (T)filter(image);
}

double	countnans(const ImagePtr& image, const Subgrid grid = Subgrid());
double	countnansrel(const ImagePtr& image, const Subgrid grid = Subgrid());

/**
 * \brief Filter that finds the largest value of all pixels
 */
template<typename T, typename S>
class Max : public PixelTypeFilter<T, S> {
public:
	Max() { }
	virtual	S	filter(const astro::image::Image<T>& image,
				const Subgrid grid = Subgrid());
	virtual T	operator()(const astro::image::Image<T>& image,
				const Subgrid grid = Subgrid());
};

template<typename T, typename S>
S	Max<T, S>::filter(const astro::image::Image<T>& image,
			const Subgrid grid) {
	return (S)this->operator()(image, grid);
}

template<typename T, typename S>
T	Max<T, S>::operator()(const astro::image::Image<T>& image,
			const Subgrid grid) {
	T	result = 0;
	for (unsigned int x = grid.origin.x; x < image.size.width;
		x += grid.size.width) {
		for (unsigned int y = grid.origin.y;
			y < image.size.height; y += grid.size.height) {
			T	v = image.pixel(x, y);
			if (v != v) continue; // skip NaNs
			if (v > result) {
				result = image.pixel(x, y);
			}
		}
	}
	return result;
}

double	max(const ImagePtr& image, const Subgrid grid = Subgrid());
double	maxrel(const ImagePtr& image, const Subgrid grid = Subgrid());

/**
 * \brief Filter that finds the smalles value of all pixels
 */
template<typename T, typename S>
class Min : public PixelTypeFilter<T, S> {
public:
	Min() { }
	virtual	S	filter(const astro::image::Image<T>& image,
				const Subgrid grid = Subgrid());
	virtual T	operator()(const astro::image::Image<T>& image,
				const Subgrid grid = Subgrid());
};

template<typename T, typename S>
S	Min<T, S>::filter(const astro::image::Image<T>& image,
			const Subgrid grid) {
	return (S) this->operator()(image, grid);
}

template<typename T, typename S>
T	Min<T, S>::operator()(const astro::image::Image<T>& image,
			const Subgrid grid) {
	T	result = std::numeric_limits<T>::max();
	for (unsigned int x = grid.origin.x; x < image.size.width;
		x += grid.size.width) {
		for (unsigned int y = grid.origin.y;
			y < image.size.height; y += grid.size.height) {
			T	v = image.pixel(x, y);
			if (v != v) continue; // skip NaNs
			if (v < result) {
				result = image.pixel(x, y);
			}
		}
	}
	return result;
}

double	min(const ImagePtr& image, const Subgrid grid = Subgrid());
double	minrel(const ImagePtr& image, const Subgrid grid = Subgrid());

/**
 * \brief Filter that finds the mean of an image
 */
template<typename T, typename S>
class Mean : public PixelTypeFilter<T, S> {
	bool	relative;
public:
	Mean(bool _relative = false) : relative(_relative) {
	}
	virtual S	filter(const astro::image::Image<T>& image,
				const Subgrid grid = Subgrid());
	virtual T	operator()(const Image<T>& image,
				const Subgrid grid = Subgrid());
};

template<typename T, typename S>
S	Mean<T, S>::filter(const astro::image::Image<T>& image,
			const Subgrid grid) {
	S	sum = 0;
	size_t	counter = 0;
	bool	check_nan = std::numeric_limits<T>::has_quiet_NaN;
	for (unsigned int x = grid.origin.x; x < image.size.width;
		x += grid.size.width) {
		for (unsigned int y = grid.origin.y;
			y < image.size.height; y += grid.size.height) {
			T	v = image.pixel(x, y);
			if ((check_nan) && (v != v))
				continue;
			sum += v;
			counter++;
		}
	}
	S	result = sum / counter;
	return result;
}

template<typename T, typename S>
T	Mean<T, S>::operator()(const Image<T>& image,
			const Subgrid grid) {
	return (T)filter(image, grid);
}

double	mean(const astro::image::ImagePtr& image, const Subgrid grid = Subgrid());
double	meanrel(const astro::image::ImagePtr& image, const Subgrid grid = Subgrid());

/**
 * \brief Filter that finds the variance of an image
 */
template<typename T, typename S>
class Variance : public Mean<T, S> {
public:
	Variance() { }
	virtual S	filter(const astro::image::Image<T>& image,
				const Subgrid grid = Subgrid());
	virtual T	operator()(const Image<T>& image,
				const Subgrid grid = Subgrid());
};

template<typename T, typename S>
S	Variance<T, S>::filter(const astro::image::Image<T>& image,
			const Subgrid grid) {
	S	m = Mean<T, S>::filter(image, grid);
	// the rest of the code is concerned with computing the
	// quadratic mean

	S	sum = 0;
	size_t	counter = 0;
	bool	check_nan = std::numeric_limits<T>::has_quiet_NaN;
	for (unsigned int x = grid.origin.x; x < image.size.width;
		x += grid.size.width) {
		for (unsigned int y = grid.origin.y;
			y < image.size.height; y += grid.size.height) {
			T	v = image.pixel(x, y);
			// skip NaNs
			if ((check_nan) && (v != v))
				continue;
			
			sum += (v - m) * (v - m);
			counter++;
		}
	}
	S	var = sum / counter;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "var = %f", var);
	return var;
}

template<typename T, typename S>
T	Variance<T, S>::operator()(const Image<T>& image, const Subgrid grid) {
	return (T)filter(image, grid);
}

/**
 * \brief Filters that finds the mean of the various color channels
 */
template<typename T, typename S>
class MatrixMean : public Mean<T, S> {
protected:
	typedef enum color_e {
		R = 0, Gr = 1, B = 2, Gb = 3
	} color_type;
	color_type	color;
public:
	MatrixMean(color_type _color) : color(_color) { }
	virtual S	mean(const astro::image::Image<T>& image) {
		if (image.mosaic & 0x8) {
			throw std::logic_error("not a mosaic image");
		}
		unsigned int	dx =  image.mosaic       & 0x1;
		unsigned int	dy = (image.mosaic >> 1) & 0x1;
		switch (color) {
		case R:
			break;
		case Gr:
			dx ^= 0x1;
			break;
		case B:
			dx ^= 0x1;
			dy ^= 0x1;
			break;
		case Gb:
			dy ^= 0x1;
			break;
		}
		S	sum = 0;
		unsigned long	counter = 0;
		for (unsigned int x = dx; x < image.size.width; x += 2) {
			for (unsigned int y = dy; y < image.size.height; y += 2) {
				sum += image.pixel(x, y);
				counter++;
			}
		}
		return sum / counter;
	}
};

template<typename T, typename S>
class MeanR : public MatrixMean<T, S> {
public:
	MeanR() : MatrixMean<T, S>(MatrixMean<T, S>::R) { };
};

template<typename T, typename S>
class MeanGr : public MatrixMean<T, S> {
public:
	MeanGr() : MatrixMean<T, S>(MatrixMean<T, S>::Gr) { };
};

template<typename T, typename S>
class MeanB : public MatrixMean<T, S> {
public:
	MeanB() : MatrixMean<T, S>(MatrixMean<T, S>::B) { };
};

template<typename T, typename S>
class MeanGb : public MatrixMean<T, S> {
public:
	MeanGb() : MatrixMean<T, S>(MatrixMean<T, S>::Gb) { };
};

/**
 * \brief Filter that finds the median of an image
 */

template<typename T, typename S>
class Median : public PixelTypeFilter<T, S> {
	enum { N = 4 };
	T	upper_limit;
	T	lower_limit;

	T	median(const astro::image::Image<T>& image,
			const T& left, const T& right, const Subgrid grid);
public:
	Median() { }

	virtual T	operator()(const Image<T>& image,
				const Subgrid grid = Subgrid());
	virtual S	filter(const Image<T>& image,
				const Subgrid grid = Subgrid());
};

template<typename T, typename S>
T	Median<T, S>::operator()(const Image<T>& image, const Subgrid grid) {
	// compute the maximum value that we have to consider.
	// this depends on whether we have a floating point type,
	// in which case we have to compute the maximum and minimum,
	// or whether we have an integer type, in which case we
	// can use std::numeric_limits
	T	lower_limit;
	T	upper_limit;
	// 
	if (std::numeric_limits<T>::is_integer) {
		lower_limit = 0;
		upper_limit = std::numeric_limits<T>::max();
	} else {
		Min<T, S>	minfilter;
		lower_limit = minfilter(image, grid);
		Max<T, S>	maxfilter;
		upper_limit = maxfilter(image, grid);
	}
	T	result = median(image, lower_limit, upper_limit, grid);
	return result;
}

template<typename T, typename S>
S	Median<T, S>::filter(const Image<T>& image, const Subgrid grid) {
	return (S)this->operator()(image, grid);
}

template<typename T, typename S>
T	Median<T, S>::median(const astro::image::Image<T>& image,
		const T& left, const T& right, const Subgrid grid) {
	size_t	pixels = image.size.pixels / grid.volume();
#if 0
std::cout << "left: " << (unsigned int)left << ", right: "
	<< (unsigned int)right << std::endl;
#endif
	unsigned long	count[N + 1];
	T	limits[N + 1];
	int	delta = 0;
	// reset all the limit values 
	for (unsigned int i = 0; i < N + 1; i++) {
		count[i] = 0;
		limits[i] = left + (i * (right - left)) / N;
		if (i > 0) {
			int	d = limits[i] - limits[i - 1];
			if (d > delta) {
				delta = d;
			}
		}
	}

	// count the number of values 
	for (unsigned int x = grid.origin.x; x < image.size.width;
		x += grid.size.width) {
		for (unsigned int y = grid.origin.y;
			y < image.size.height; y += image.size.height) {
			T	v = image.pixel(x, y);
			for (unsigned int i = 0; i < N + 1; i++) {
				if (v <= limits[i]) {
					count[i]++;
				}
			}
		}
	}

	// terminal condition, if the delta <= 1, then we have
	// enough resolution to determine the median
	if (delta == 0) {
		return limits[0];
	}
	if (delta <= 1) {
		for (unsigned int i = 1; i < N + 1; i++) {
			if (((2 * count[i - 1]) < pixels)
				&& (pixels <= (2 * count[i]))) {
				return (limits[i - 1] + limits[i]) / 2;
			}
		}
	}

	// now we know how many values are in each bucket
#if 0
	for (int i = 0; i < N + 1; i++) {
		std::cout << "limit: " << limits[i]
			<< ", count: " << count[i] << std::endl;
	}
#endif
	// now find out in which interval we have to expect the
	// median
	
	if ((2 * count[N]) < pixels) {
		return median(image, limits[N], upper_limit, grid);
	}
	if (pixels <= (2 * count[0])) {
		return median(image, 0, limits[0], grid);
	}
	for (unsigned int i = 1; i < N + 1; i++) {
		if (((2 * count[i - 1]) < pixels)
			&& (pixels <= (2 * count[i]))) {
			return median(image, limits[i - 1], limits[i], grid);
		}
	}
	throw std::logic_error("error in median computation");
}

double	median(const ImagePtr& image, const Subgrid grid = Subgrid());

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

} // namespace filter
} // namespace image
} // namespace astro

#endif /* _AstroFilter_h */
