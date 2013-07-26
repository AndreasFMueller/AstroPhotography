/*
 * AstroFilter.h -- filters to apply to images 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroFilter_h
#define _AstroFilter_h

#include <AstroImage.h>
#include <AstroAdapter.h>
#include <limits>
#include <AstroDebug.h>

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
template<typename T, typename S>
class PixelTypeFilter {
public:
	virtual S	filter(const ConstImageAdapter<T>& image) = 0;
	virtual T	operator()(const ConstImageAdapter<T>& image) = 0;
};

/**
 * \brief Filter to count NaNs
 */
template<typename T, typename S>
class CountNaNs : public PixelTypeFilter<T, S> {
public:
	CountNaNs() { }
	virtual S	filter(const ConstImageAdapter<T>& image);
	virtual T	operator()(const ConstImageAdapter<T>& image);
};

template<typename T, typename S>
S	CountNaNs<T, S>::filter(const ConstImageAdapter<T>& image) {
	S	result = 0;
	ImageSize	size = image.getSize();
	for (unsigned int x = 0; x < size.getWidth(); x++) {
		for (unsigned int y = 0; y < size.getHeight(); y++) {
			T	v = image.pixel(x, y);
			if (v != v) {
				result += 1;
			}
		}
	}
	return result;
}

template<typename T, typename S>
T	CountNaNs<T, S>::operator()(const ConstImageAdapter<T>& image) {
	return (T)filter(image);
}

double	countnans(const ImagePtr& image);
double	countnansrel(const ImagePtr& image);

/**
 * \brief Filter that finds the largest value of all pixels
 */
template<typename T, typename S>
class Max : public PixelTypeFilter<T, S> {
public:
	Max() { }
	virtual	S	filter(const ConstImageAdapter<T>& image);
	virtual T	operator()(const ConstImageAdapter<T>& image);
};

template<typename T, typename S>
S	Max<T, S>::filter(const ConstImageAdapter<T>& image) {
	return (S)this->operator()(image);
}

template<typename T, typename S>
T	Max<T, S>::operator()(const ConstImageAdapter<T>& image) {
	T	result = 0;
	ImageSize	size = image.getSize();
	for (unsigned int x = 0; x < size.getWidth(); x++) {
		for (unsigned int y = 0; y < size.getHeight(); y++) {
			T	v = image.pixel(x, y);
			if (v != v) continue; // skip NaNs
			if (v > result) {
				result = image.pixel(x, y);
			}
		}
	}
	return result;
}

/**
 * \brief Filter that finds the smalles value of all pixels
 */
template<typename T, typename S>
class Min : public PixelTypeFilter<T, S> {
public:
	Min() { }
	virtual	S	filter(const ConstImageAdapter<T>& image);
	virtual T	operator()(const ConstImageAdapter<T>& image);
};

template<typename T, typename S>
S	Min<T, S>::filter(const ConstImageAdapter<T>& image) {
	return (S) this->operator()(image);
}

template<typename T, typename S>
T	Min<T, S>::operator()(const ConstImageAdapter<T>& image) {
	T	result = std::numeric_limits<T>::max();
	ImageSize	size = image.getSize();
	for (unsigned int x = 0; x < size.getWidth(); x++) {
		for (unsigned int y = 0; y < size.getHeight(); y++) {
			T	v = image.pixel(x, y);
			if (v != v) continue; // skip NaNs
			if (v < result) {
				result = v;
			}
		}
	}
	return result;
}

/**
 * \brief Filter that finds the mean of an image
 */
template<typename T, typename S>
class Mean : public PixelTypeFilter<T, S> {
	bool	relative;
public:
	Mean(bool _relative = false) : relative(_relative) {
	}
	virtual S	filter(const ConstImageAdapter<T>& image);
	virtual T	operator()(const ConstImageAdapter<T>& image);
};

template<typename T, typename S>
S	Mean<T, S>::filter(const ConstImageAdapter<T>& image) {
	ImageSize	size = image.getSize();
	S	sum = 0;
	size_t	counter = 0;
	bool	check_nan = std::numeric_limits<T>::has_quiet_NaN;
	for (unsigned int x = 0; x < size.getWidth(); x++) {
		for (unsigned int y = 0; y < size.getHeight(); y++) {
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
T	Mean<T, S>::operator()(const ConstImageAdapter<T>& image) {
	return (T)filter(image);
}

/**
 * \brief Filter that finds the variance of an image
 */
template<typename T, typename S>
class Variance : public Mean<T, S> {
public:
	Variance() { }
	virtual S	filter(const ConstImageAdapter<T>& image);
	virtual T	operator()(const ConstImageAdapter<T>& image);
};

template<typename T, typename S>
S	Variance<T, S>::filter(const ConstImageAdapter<T>& image) {
	S	m = Mean<T, S>::filter(image);
	// the rest of the code is concerned with computing the
	// quadratic mean

	S	sum = 0;
	size_t	counter = 0;
	bool	check_nan = std::numeric_limits<T>::has_quiet_NaN;
	ImageSize	size = image.getSize();
	for (unsigned int x = 0; x < size.getWidth(); x++) {
		for (unsigned int y = 0; y < size.getHeight(); y++) {
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
T	Variance<T, S>::operator()(const ConstImageAdapter<T>& image) {
	return (T)filter(image);
}

/**
 * \brief Filters that finds the mean of the various color channels
 */
template<typename T, typename S>
class MosaicMean : public Mean<T, S> {
protected:
	typedef enum color_e {
		R = 0, Gr = 1, B = 2, Gb = 3
	} color_type;
	color_type	color;
	ImagePoint	origin(const ImageBase::mosaic_type& mosaic) {
		unsigned int	dx =  mosaic       & 0x1;
		unsigned int	dy = (mosaic >> 1) & 0x1;
		ImagePoint	o(dx, dy);
		switch (color) {
		case R:
			break;
		case Gr:
			o.x ^= 0x1;
			break;
		case B:
			o.x ^= 0x1;
			o.y ^= 0x1;
			break;
		case Gb:
			o.y ^= 0x1;
			break;
		}
		return o;
	}
public:
	MosaicMean(color_type _color) : color(_color) { }
	virtual S	mean(const ConstImageAdapter<T>& image,
				const ImageBase::mosaic_type& mosaic) {
		if (mosaic & 0x8) {
			throw std::logic_error("not a mosaic image");
		}
		Subgrid	grid(origin(mosaic), ImageSize(2, 2));
		ConstSubgridAdapter<T>	subimage(image, grid);
		Mean<T, S>	m;
		return m.filter(image);
	}
};

template<typename T, typename S>
class MeanR : public MosaicMean<T, S> {
public:
	MeanR() : MosaicMean<T, S>(MosaicMean<T, S>::R) { }
};

template<typename T, typename S>
class MeanGr : public MosaicMean<T, S> {
public:
	MeanGr() : MosaicMean<T, S>(MosaicMean<T, S>::Gr) { };
};

template<typename T, typename S>
class MeanB : public MosaicMean<T, S> {
public:
	MeanB() : MosaicMean<T, S>(MosaicMean<T, S>::B) { };
};

template<typename T, typename S>
class MeanGb : public MosaicMean<T, S> {
public:
	MeanGb() : MosaicMean<T, S>(MosaicMean<T, S>::Gb) { };
};

/**
 * \brief Filter that finds the median of an image
 */

template<typename T, typename S>
class Median : public PixelTypeFilter<T, S> {
	enum { N = 4 };
	T	upper_limit;
	T	lower_limit;

	T	median(const ConstImageAdapter<T>& image,
			const T& left, const T& right);
public:
	Median() { }

	virtual T	operator()(const ConstImageAdapter<T>& image);
	virtual S	filter(const ConstImageAdapter<T>& image);
};

template<typename T, typename S>
T	Median<T, S>::operator()(const ConstImageAdapter<T>& image) {
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
		lower_limit = minfilter(image);
		Max<T, S>	maxfilter;
		upper_limit = maxfilter(image);
	}
	T	result = median(image, lower_limit, upper_limit);
	return result;
}

template<typename T, typename S>
S	Median<T, S>::filter(const ConstImageAdapter<T>& image) {
	return (S)this->operator()(image);
}

template<typename T, typename S>
T	Median<T, S>::median(const ConstImageAdapter<T>& image,
		const T& left, const T& right) {
	ImageSize	size = image.getSize();
	size_t	pixels = size.getPixels();
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
	for (unsigned int x = 0; x < size.getWidth(); x++) {
		for (unsigned int y = 0; y < size.getHeight(); y++) {
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
		return median(image, limits[N], upper_limit);
	}
	if (pixels <= (2 * count[0])) {
		return median(image, 0, limits[0]);
	}
	for (unsigned int i = 1; i < N + 1; i++) {
		if (((2 * count[i - 1]) < pixels)
			&& (pixels <= (2 * count[i]))) {
			return median(image, limits[i - 1], limits[i]);
		}
	}
	throw std::logic_error("error in median computation");
}

/**
 * \brief Figure of Merit for autofocus
 *
 * This filter computes the integral of the value times the laplacian
 * of the image function. This is the L^2-norm of the first derivative.
 * The larger in absolute value, the better the focus. 
 */
template<typename Pixel>
class FocusFOM : public PixelTypeFilter<Pixel, double> {
	bool	diagonal;
	double	scale;
public:
	FocusFOM(bool _diagonal = false, double _scale = 1)
		: diagonal(_diagonal), scale(_scale) {
	}

	virtual double	filter(const ConstImageAdapter<Pixel>& image) {
		FocusFOMAdapter<Pixel>	foa(image, diagonal);
		ImageSize	size = foa.getSize();
		double	result = 0;
		for (size_t x = 0; x < size.getWidth(); x++) {
			for (size_t y = 0; y < size.getHeight(); y++) {
				double	l = foa.pixel(x, y);
				// skip NaNs
				if (l == l) {
					result += l;
				}
			}
		}
		return scale * result;
	}

	virtual Pixel	operator()(const ConstImageAdapter<Pixel>& image) {
		Pixel	result = this->filter(image);
		return result;
	}
};

/**
 * \brief Image masking operations
 *
 * Masking operations are used to apply windowing functions or to black out
 * parts of an image the we don't want to see. 
 * a common base class.
 */
template<typename Pixel>
class Mask {
	MaskingFunction&	maskingfunction;
public:
	Mask(MaskingFunction& _maskingfunction)
		: maskingfunction(_maskingfunction) {
	}
	void	operator()(Image<Pixel>& image);
};

template<typename Pixel>
void    Mask<Pixel>::operator()(Image<Pixel>& image) {
	for (size_t x = 0; x < image.size.getWidth(); x++) {
		for (size_t y = 0; y < image.size.getHeight(); y++) {
			Pixel   v = image.pixel(x, y);
			v = maskingfunction(x, y) * v;
			image.pixel(x, y) = v;
		}
	}
}

} // namespace filter
} // namespace image
} // namespace astro

#endif /* _AstroFilter_h */
