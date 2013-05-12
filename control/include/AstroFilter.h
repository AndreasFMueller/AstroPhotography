/*
 * AstroFilter.h -- filters to apply to images 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroFilter_h
#define _AstroFilter_h

namespace astro {
namespace image {
namespace filter {

/**
 * \brief Filters that return a single value of the same type as the image.
 *
 * This type of filter cannot be used to compute values from the image
 * that don't fit in the pixel type. An example of such a value would be
 * the mean value. There is a Mean filter derived from this type but in its
 * basic form it computes the integer rounded version.
 */
template<typename T>
class PixelTypeFilter {
public:
	virtual T	operator()(const Image<T>& image) = 0;
};

/**
 * \brief Filter that finds the largest value of all pixels
 */
template<typename T>
class Max : public PixelTypeFilter<T> {
public:
	Max() { }
	virtual T	operator()(const Image<T>& image) {
		T	result = 0;
		for (unsigned int i = 0; i < image.size.pixels; i++) {
			if (image.pixels[i] > result) {
				result = image.pixels[i];
			}
		}
		return result;
	}
};

/**
 * \brief Filter that fines the smalles value of all pixels
 */
template<typename T>
class Min : public PixelTypeFilter<T> {
public:
	Min() { }
	virtual T	operator()(const Image<T>& image) {
		T	result = std::numeric_limits<T>::max();
		for (unsigned int i = 0; i < image.size.pixels; i++) {
			if (image.pixels[i] < result) {
				result = image.pixels[i];
			}
		}
		return result;
	}
};

/**
 * \brief Filter that finds the mean of an image
 */
template<typename T, typename S>
class Mean : public PixelTypeFilter<T> {
public:
	Mean() { }
	S	mean(const Image<T>& image) {
		S	sum = 0;
		for (unsigned int i = 0; i < image.size.pixels; i++) {
			sum += image.pixels[i];
		}
		return sum / image.size.pixels;
	}
	virtual T	operator()(const Image<T>& image) {
		return (T)mean(image);
	}
};

/**
 * \brief Filter that finds the median of an image
 */

template<typename T>
class Median : public PixelTypeFilter<T> {
	enum { N = 4 };
	T	upper_limit;
	T	lower_limit;

	T	median(const Image<T>& image, const T& left, const T& right) {
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
		for (unsigned int p = 0; p < image.size.pixels; p++) {
			T	v = image.pixels[p];
			for (unsigned int i = 0; i < N + 1; i++) {
				if (v <= limits[i]) {
					count[i]++;
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
				if (((2 * count[i - 1]) < image.size.pixels)
					&& (image.size.pixels <= (2 * count[i]))) {
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
		
		if ((2 * count[N]) < image.size.pixels) {
			return median(image, limits[N], upper_limit);
		}
		if (image.size.pixels <= (2 * count[0])) {
			return median(image, 0, limits[0]);
		}
		for (unsigned int i = 1; i < N + 1; i++) {
			if (((2 * count[i - 1]) < image.size.pixels)
				&& (image.size.pixels <= (2 * count[i]))) {
				return median(image, limits[i - 1], limits[i]);
			}
		}
		throw std::logic_error("error in median computation");
	}
public:
	Median() { }
	virtual T	operator()(const Image<T>& image) {
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
			Min<T>	minfilter;
			lower_limit = minfilter(image);
			Max<T>	maxfilter;
			upper_limit = maxfilter(image);
		}
		T	result = median(image, lower_limit, upper_limit);
		return result;
	}
};

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
	virtual void	operator()(Image<T>& image) {
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
	virtual void	operator()(Image<T>& image) {
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

	virtual void	operator()(Image<T>& image) {
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
