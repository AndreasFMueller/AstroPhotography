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
 * \brief Subgrid specification
 *
 * Some filter operations only happen on a subgrid, specified by this 
 * class
 */
class	Subgrid {
public:
	ImagePoint	gridorigin;
	ImageSize	gridsize;
	Subgrid() : gridorigin(ImagePoint(0, 0)), gridsize(ImageSize(1, 1)) { }
	Subgrid(const ImagePoint& origin, const ImageSize& size)
		: gridorigin(origin), gridsize(size) { }
	Subgrid(const Subgrid& other) : gridorigin(other.gridorigin),
		gridsize(other.gridsize) {
	}
	size_t	volume() const { return gridsize.width * gridsize.height; }
};

/**
 * \brief Subgrid Adapter
 *
 * The subgrid adapter mediates access to the images pixel taking the
 * subgrid into account. This allows to formulate the filters for a
 * subgrid as if they were just the image
 */
template <typename Pixel>
class SubgridAdapter : public Subgrid {
	Image<Pixel>&	image;
public:
	ImageSize	size;
private:
	void	computesize() {
		// compute the resulting image size
		size = ImageSize(
			(image.size.width - gridorigin.x) / gridsize.width,
			(image.size.height - gridorigin.y) / gridsize.height
		);
	}
public:
	SubgridAdapter(const ImagePoint& gridorigin, const ImageSize& gridsize,
		Image<Pixel>& _image)
		: Subgrid(gridorigin, gridsize), image(_image) {
		computesize();
	}
	SubgridAdapter(const Subgrid& subgrid, Image<Pixel>& _image)
		: Subgrid(subgrid), image(_image) {
		computesize();
	}
	Pixel&	pixel(size_t x, size_t y) {
		return image.pixel(gridorigin.x + x * gridsize.width,
			gridorigin.y + y * gridsize.height);
	}
};

template <typename Pixel>
class ConstSubgridAdapter : public Subgrid {
	const Image<Pixel>&	image;
public:
	ImageSize	size;
private:
	void	computesize() {
		// compute the resulting image size
		size = ImageSize(
			(image.size.width - gridorigin.x) / gridsize.width,
			(image.size.height - gridorigin.y) / gridsize.height
		);
	}
public:
	ConstSubgridAdapter(const ImagePoint& gridorigin,
		const ImageSize& gridsize,
		const Image<Pixel>& _image)
		: Subgrid(gridorigin, gridsize), image(_image) {
		computesize();
	}
	ConstSubgridAdapter(const Subgrid& subgrid, const Image<Pixel>& _image)
		: Subgrid(subgrid), image(_image) {
		computesize();
	}
	Pixel	pixel(size_t x, size_t y) const {
		return image.pixel(gridorigin.x + x * gridsize.width,
			gridorigin.y + y * gridsize.height);
	}
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
	ConstSubgridAdapter<T>	sga(grid, image);
	for (unsigned int x = 0; x < sga.size.width; x++) {
		for (unsigned int y = 0; y < sga.size.height; y++) {
			T	v = sga.pixel(x, y);
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
	ConstSubgridAdapter<T>	sga(grid, image);
	for (unsigned int x = 0; x < sga.size.width; x++) {
		for (unsigned int y = 0; y < sga.size.height; y++) {
			T	v = sga.pixel(x, y);
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
	const ConstSubgridAdapter<T>	sga(grid, image);
	for (unsigned int x = 0; x < sga.size.width; x++) {
		for (unsigned int y = 0; y < sga.size.height; y++) {
			T	v = sga.pixel(x, y);
			if (v != v) continue; // skip NaNs
			if (v < result) {
				result = v;
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
	const ConstSubgridAdapter<T>	sga(grid, image);
	S	sum = 0;
	size_t	counter = 0;
	bool	check_nan = std::numeric_limits<T>::has_quiet_NaN;
	for (unsigned int x = 0; x < sga.size.width; x++) {
		for (unsigned int y = 0; y < sga.size.height; y++) {
			T	v = sga.pixel(x, y);
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
	const ConstSubgridAdapter<T>	sga(grid, image);
	bool	check_nan = std::numeric_limits<T>::has_quiet_NaN;
	for (unsigned int x = 0; x < sga.size.width; x++) {
		for (unsigned int y = 0; y < sga.size.height; y++) {
			T	v = sga.pixel(x, y);
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
		if (image.getMosaicType() & 0x8) {
			throw std::logic_error("not a mosaic image");
		}
		unsigned int	dx =  image.getMosaicType()       & 0x1;
		unsigned int	dy = (image.getMosaicType() >> 1) & 0x1;
		ImagePoint	origin(dx, dy);
		switch (color) {
		case R:
			break;
		case Gr:
			origin.x ^= 0x1;
			break;
		case B:
			origin.x ^= 0x1;
			origin.y ^= 0x1;
			break;
		case Gb:
			origin.y ^= 0x1;
			break;
		}
		Subgrid	grid(origin, ImageSize(2, 2));
		Mean<T, S>	m;
		return m.filter(image, grid);
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
	const ConstSubgridAdapter<T>	sga(grid, image);
	for (unsigned int x = 0; x < sga.size.width; x++) {
		for (unsigned int y = 0; y < sga.size.height; y++) {
			T	v = sga.pixel(x, y);
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
 * \brief Figure of Merit for autofocus
 *
 * This filter computes the integral of the value times the laplacian
 * of the image function. This is the L^2-norm of the first derivative.
 * The larger in absolute value, the better the focus. 
 */
template<typename Pixel, typename S>
class FocusFOM : public PixelTypeFilter<Pixel, S> {
	/**
	 * \brief Direction for computation of the laplacian
	 *
	 * For Bayer RGB images, it is preferable to compute the laplacian
	 * diagonally.
	 */
	bool	diagonal;
	double	scale;
	S	laplacian(const ConstSubgridAdapter<Pixel>& image,
			size_t x, size_t y) const {
		S	result = -4 * image.pixel(x, y);
		if (diagonal) {
			result += image.pixel(x - 1, y);
			result += image.pixel(x + 1, y);
			result += image.pixel(x, y - 1);
			result += image.pixel(x, y + 1);
		} else {
			result += image.pixel(x - 1, y - 1);
			result += image.pixel(x + 1, y - 1);
			result += image.pixel(x - 1, y + 1);
			result += image.pixel(x + 1, y + 1);
		}
		return result / 4;
	}
public:
	FocusFOM(bool _diagonal = false, double _scale = 1)
		: diagonal(_diagonal), scale(_scale) {
		if (diagonal) {
			scale /= sqrt(2.);
		}
	}

	virtual S	filter(const astro::image::Image<Pixel>& image,
				Subgrid subgrid = Subgrid()) {
		S	result = 0;
		ConstSubgridAdapter<Pixel>	sga(subgrid, image);
		for (size_t x = 1; x < sga.size.width - 1; x++) {
			for (size_t y = 1; y < sga.size.height - 1; y++) {
				S	l = laplacian(sga, x, y)
						* sga.pixel(x, y);
				// skip NaNs
				if (l == l) {
					result -= l;
				}
			}
		}
		return scale * result;
	}

	virtual Pixel	operator()(const astro::image::Image<Pixel>& image,
				Subgrid subgrid = Subgrid()) {
		Pixel	result = this->filter(image, subgrid);
		return result;
	}
};

double	focusFOM(const ImagePtr& image, const bool diagonal = false,
		const Subgrid grid = Subgrid());

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

class HanningMaskingFunction : public MaskingFunction {
protected:
	double	hanningradius;
	double	hanningfunction(double x) const;
public:
	HanningMaskingFunction(double hanningradius);
};

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

class CircleFunction : public HanningMaskingFunction {
protected:
	ImagePoint	center;
	double	radius;
public:
	CircleFunction(const ImagePoint& center, double radius,
		double hanningradius = 0);
	virtual double	operator()(size_t x, size_t y) const;
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
	for (size_t x = 0; x < image.size.width; x++) {
		for (size_t y = 0; y < image.size.height; y++) {
			Pixel   v = image.pixel(x, y);
			v = maskingfunction(x, y) * v;
			image.pixel(x, y) = v;
		}
	}
}

void	mask(MaskingFunction& maskingfunction, ImagePtr image);

} // namespace filter
} // namespace image
} // namespace astro

#endif /* _AstroFilter_h */
