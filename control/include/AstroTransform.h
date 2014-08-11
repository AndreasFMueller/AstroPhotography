/*
 * AstroTransform.h -- transformation of images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroTransform_h
#define _AstroTransform_h

#include <AstroTypes.h>
#include <AstroImage.h>
#include <AstroFormat.h>

namespace astro {
namespace image {
namespace transform {

/**
 * \brief A translation adapter applies a trnslation to an image
 */
template<typename Pixel>
class TranslationAdapter : public ConstImageAdapter<Pixel> {
	const ConstImageAdapter<Pixel>& image;
	Point	translation;
	int	tx, ty;
	double	weights[4];
public:
	TranslationAdapter(const ConstImageAdapter<Pixel>& image,
		const Point& translation);
	virtual Pixel	pixel(unsigned int x, unsigned int y) const;
};

template<typename Pixel>
TranslationAdapter<Pixel>::TranslationAdapter(
	const ConstImageAdapter<Pixel>& _image, const Point& _translation) 
	: ConstImageAdapter<Pixel>(_image.getSize()),
	  image(_image), translation(_translation) {
	tx = floor(translation.x());
	ty = floor(translation.y());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tx = %d, ty = %d", tx, ty);
	double	wx = translation.x() - tx;
	double	wy = translation.y() - ty;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "wx = %f, wy = %f", wx, wy);
	// compute the weights
	weights[0] = wx * wy;
	weights[1] = (1 - wx) * wy;
	weights[2] = wx * (1 - wy);
	weights[3] = (1 - wx) * (1 - wy);
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"w[0] = %f, w[1] = %f, w[2] = %f, w[3] = %f",
		weights[0], weights[1], weights[2], weights[3]);
}

template<typename Pixel>
Pixel	TranslationAdapter<Pixel>::pixel(unsigned int x, unsigned int y) const {
	Pixel	a[4];
	ImageSize	size = ConstImageAdapter<Pixel>::getSize();
	// lower left corner
	if (size.contains(-tx + x - 1, -ty + y - 1)) {
		a[0] = image.pixel(-tx + x - 1, -ty + y - 1);
	} else {
		a[0] = Pixel(0);
	}
	// lower right corner
	if (size.contains(-tx + x    , -ty + y - 1)) {
		a[1] = image.pixel(-tx + x    , -ty + y - 1);
	} else {
		a[1] = Pixel(0);
	}
	// upper left corner
	if (size.contains(-tx + x - 1, -ty + y    )) {
		a[2] = image.pixel(-tx + x - 1, -ty + y    );
	} else {
		a[2] = Pixel(0);
	}
	// upper right corner
	if (size.contains(-tx + x    , -ty + y    )) {
		a[3] = image.pixel(-tx + x    , -ty + y    );
	} else {
		a[3] = Pixel(0);
	}
	return weighted_sum(4, weights, a);
}

ImagePtr	translate(ImagePtr source, const Point& translation);

/**
 * \brief Adapter to interpolate pixels
 *
 * If the pixel type allows NaNs, then pixels that are mapped outside the
 * original image are given NaN values. This allows e.g. the Analyzer to
 * to detect when there is no data to compute a residual.
 */
template<typename Pixel>
class PixelInterpolationAdapter : public ConstImageAdapter<Pixel> {
	const ConstImageAdapter<Pixel>&	image;
	Pixel	defaultpixel;
public:
	PixelInterpolationAdapter(const ConstImageAdapter<Pixel>& _image)
		: ConstImageAdapter<Pixel>(_image.getSize()), image(_image) {
		if (std::numeric_limits<Pixel>::has_quiet_NaN) {
			defaultpixel = std::numeric_limits<Pixel>::quiet_NaN();
		} else {
			defaultpixel = Pixel(0);
		}
	}
	Pixel	pixel(const astro::Point& t) const {
		// find out in which pixel this is located
		int     tx = floor(t.x());
		int     ty = floor(t.y());

		// compute the weights
		double  wx = t.x() - tx;
		double  wy = t.y() - ty;

		// compute the weights
		double  weights[4];
		weights[0] = (1 - wx) * (1 - wy);
		weights[1] = wx * (1 - wy);
		weights[2] = (1 - wx) * wy;
		weights[3] = wx * wy;

		// now compute the weighted sum of the pixels
		Pixel   a[4];
		ImageSize       size = ConstImageAdapter<Pixel>::getSize();

		// lower left corner
		if (size.contains(tx    , ty    )) {
			a[0] = image.pixel(tx    , ty    );
		} else {
			a[0] = defaultpixel;
		}
		// lower right corner
		if (size.contains(tx + 1, ty    )) {
			a[1] = image.pixel(tx + 1, ty    );
		} else {
			a[1] = defaultpixel;
		}
		// upper left corner
		if (size.contains(tx    , ty + 1)) {
			a[2] = image.pixel(tx    , ty + 1);
		} else {
			a[2] = defaultpixel;
		}
		// upper right corner
		if (size.contains(tx + 1, ty + 1)) {
			a[3] = image.pixel(tx + 1, ty + 1);
		} else {
			a[3] = defaultpixel;
		}
		return weighted_sum(4, weights, a);
	}
	virtual Pixel	pixel(unsigned int x, unsigned int y) const {
		return image.pixel(x, y);
	}
};

/**
 * \brief Residuals needed to analyze transforms
 */
class Residual : public std::pair<ImagePoint, Point> {
	ImagePoint	_from;
	Point	_offset;
	double	_weight;
public:
	Residual(const ImagePoint& from, const Point& offset,
		double weight = 1.)
		: _from(from), _offset(offset), _weight(weight) {
	}
	const ImagePoint&	from() const { return _from; }
	ImagePoint&	from() { return _from; }
	const Point&	offset() const { return _offset; }
	Point&	offset() { return _offset; }
	const double&	weight() const { return _weight; }
	double&	weight() { return _weight; }
	bool	invalid() const;
	bool	valid() const { return !invalid(); }
};

/**
 * \brief Abstraction of an affine transform
 */

class Transform {
	double	a[6];
public:
	// constructors
	Transform();
	Transform(const Transform& other);
	Transform(double angle, const Point& translation,
		double scalefactor = 1);
	Transform(const std::vector<Residual>& residuals);

	// check whether this is a certain type of transform
	bool	isIdentity() const;
	bool	isTranslation() const;
	bool	isRotation() const;
	bool	isHomothety() const;
	bool	isIsometry() const;
	bool	isAreaPreserving() const;
	bool	isAnglePreserving() const;
	bool	fixesOrigin() const;
	bool	operator==(const Transform& other) const;
	bool	operator!=(const Transform& other) const;

	// compute the inverse transformation
	Transform	inverse() const;

	// data access
	Point	getTranslation() const;

	// operations
	Transform	operator*(const Transform& other) const;
	Transform	operator+(const Point& translation) const;
	Transform	operator+(const astro::image::ImagePoint& translation)
				const;

	// access to the coefficients
	virtual double	operator[](int i) const;
	virtual double&	operator[](int i);

	// operating on points 
	virtual Point	operator()(const Point& point) const;

	// for debugging
	virtual std::string	toString() const;
	friend std::ostream&	operator<<(std::ostream& out,
		const Transform& transform);
};

/**
 * \brief
 */
template<typename Pixel>
class TransformAdapter : public ConstImageAdapter<Pixel> {
	const PixelInterpolationAdapter<Pixel>	image;
	Transform	transform;
	Transform	inverse;
public:
	TransformAdapter(const ImageSize& targetsize,
		const ConstImageAdapter<Pixel>& image,
		const Transform& transform);
	TransformAdapter(const ConstImageAdapter<Pixel>& image,
		const Transform& transform);
	virtual Pixel	pixel(unsigned int x, unsigned int y) const;
};

template<typename Pixel>
TransformAdapter<Pixel>::TransformAdapter(const ImageSize& targetsize,
	const ConstImageAdapter<Pixel>& _image, const Transform& _transform)
	: ConstImageAdapter<Pixel>(targetsize), image(_image),
	  transform(_transform) {
	inverse = transform.inverse();
}

template<typename Pixel>
TransformAdapter<Pixel>::TransformAdapter(
	const ConstImageAdapter<Pixel>& _image, const Transform& _transform)
	: ConstImageAdapter<Pixel>(_image.getSize()),
	  image(_image), transform(_transform) {
	inverse = transform.inverse();
}

template<typename Pixel>
Pixel	TransformAdapter<Pixel>::pixel(unsigned int x, unsigned int y) const {
	// compute the image if the point x, y under the inverse transform
	Point	t = inverse(Point(x, y));
	return image.pixel(t);
}

ImagePtr	transform(ImagePtr image, const Transform& transform);

/**
 * \brief Find a translation between two images
 *
 * This method uses Fourier transform and phase correlation to find the
 * (necessarily small) translation with subpixel accuracy.
 */
class PhaseCorrelator {
	double	value(const double *a, const astro::image::ImageSize& size,
			int x, int y) const;
	Point	centroid(const double *a, const astro::image::ImageSize& size,
			const astro::Point& center,
			unsigned int k = 2) const;
	bool	hanning;
public:
	PhaseCorrelator(bool _hanning = true) : hanning(_hanning) { }
	std::pair<Point, double>	operator()(
		const ConstImageAdapter<double>& fromimage,
		const ConstImageAdapter<double>& toimage);
};

/**
 * \brief Analysis of a transformation and get a list of 
 */
class Analyzer {
	const ConstImageAdapter<double>& baseimage;
	unsigned int	spacing;
	unsigned int	patchsize;
public:
	Analyzer(const ConstImageAdapter<double>& _baseimage,
		unsigned int _spacing = 128, unsigned int _patchsize = 128)
                : baseimage(_baseimage),
                  spacing(_spacing), patchsize(_patchsize)  {
	}
        std::vector<Residual>	operator()(const ConstImageAdapter<double>& image) const;
};

/**
 * \brief Find a general transformation between
 */
class TransformAnalyzer : public Analyzer {
public:
	TransformAnalyzer(const ConstImageAdapter<double>& _baseimage,
		unsigned int _spacing = 128, unsigned int _patchsize = 128)
		: Analyzer(_baseimage, _spacing, _patchsize) { }
	Transform	transform(const ConstImageAdapter<double>& image) const;
};

} // namespace transform
} // namespace image
} // namespace astro

#endif /* _AstroTransform_h */
