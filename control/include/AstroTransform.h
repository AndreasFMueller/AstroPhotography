/*
 * AstroTransform.h -- transformation of images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroTransform_h
#define _AstroTransform_h

#include <AstroImage.h>
#include <Format.h>

namespace astro {
namespace image {
namespace transform {

/**
 * \brief Point with noninteger coordinates
 *
 * Such points are needed when registering images.
 */

class Point {
public:
	double	x;
	double	y;
	Point() : x(0), y(0) { }
	Point(double _x, double _y) : x(_x), y(_y) { }
	Point(const astro::image::ImagePoint& point) : x(point.x), y(point.y) {}
	Point	operator+(const Point& other) const;
	Point	operator-(const Point& other) const;
	Point	operator*(double l) const;
	friend Point	operator*(double l, const Point& other);
	std::string	toString() const;
	bool	operator==(const Point& other) const;
	bool	operator!=(const Point& other) const;
};
Point	operator*(double l, const Point& other);
std::ostream&	operator<<(std::ostream& out, const Point& other);

/**
 * \brief
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
	virtual const Pixel	pixel(unsigned int x, unsigned int y) const;
};

template<typename Pixel>
TranslationAdapter<Pixel>::TranslationAdapter(
	const ConstImageAdapter<Pixel>& _image, const Point& _translation) 
	: ConstImageAdapter<Pixel>(_image.getSize()),
	  image(_image), translation(_translation) {
	tx = floor(translation.x);
	ty = floor(translation.y);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tx = %d, ty = %d", tx, ty);
	double	wx = translation.x - tx;
	double	wy = translation.y - ty;
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
const Pixel	TranslationAdapter<Pixel>::pixel(unsigned int x, unsigned int y) const {
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
	Transform(const std::vector<Point>& frompoints,
		const std::vector<Point>& topoints);

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

	// operating on points 
	Point	operator()(const Point& point) const;

	// for debugging
	std::string	toString() const;
	friend std::ostream&	operator<<(std::ostream& out,
		const Transform& transform);
};

/**
 * \brief
 */
template<typename Pixel>
class TransformAdapter {
	const ConstImageAdapter<Pixel>& image;
	Transform	transform;
public:
	TransformAdapter(const ConstImageAdapter<Pixel>& image,
		const Transform& transform);
	virtual const Pixel	pixel(unsigned int x, unsigned int y) const;
};

template<typename Pixel>
TransformAdapter<Pixel>::TransformAdapter(
	const ConstImageAdapter<Pixel>& _image, const Transform& _transform)
	: ConstImageAdapter<Pixel>(_image.getSize()), image(_image),
	  transform(_transform) {
}

template<typename Pixel>
const Pixel	TransformAdapter<Pixel>::pixel(unsigned int x, unsigned int y) const {
	return Pixel(0);
}


/**
 * \brief Find a transformation between two images
 */
class PhaseCorrelator {
	double	value(const double *a, const astro::image::ImageSize& size,
			unsigned int x, unsigned int y) const;
	Point	centroid(const double *a, const astro::image::ImageSize& size,
			const astro::image::ImagePoint& center,
			unsigned int k = 2) const;
public:
	Point	operator()(const ConstImageAdapter<double>& fromimage,
			const ConstImageAdapter<double>& toimage);
};

} // namespace transform
} // namespace image
} // namespace astro

#endif /* _AstroTransform_h */
