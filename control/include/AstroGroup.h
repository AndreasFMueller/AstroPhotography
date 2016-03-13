/*
 * AstroGroup.h -- transformation groups for experiments with noncommutative
 *                 harmonic analysis
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _AstroGroup_h
#define _AstroGroup_h

#include <AstroTypes.h>
#include <AstroTransform.h>
#include <AstroDebug.h>

namespace astro {
namespace image {
namespace transform {

/**
 * \brief Class to model euclidean displacements of a plane
 *
 * This class is used for experiments related to noncommutative harmonic
 * analysis.
 */
class EuclideanDisplacement {
	double	_a;
	Point	_t;
public:
	double	angle() const { return _a; }
	Point	translation() const { return _t; }
private:
	double	c, s;
	void	setup();
	Point	rotate(const Point& other) const;
public:
	EuclideanDisplacement();
	EuclideanDisplacement(double angle);
	EuclideanDisplacement(Point translation);
	EuclideanDisplacement(double angle, Point translation);
	EuclideanDisplacement	inverse() const;
	EuclideanDisplacement	operator*(const EuclideanDisplacement& other)
					const;
	EuclideanDisplacement	operator/(const EuclideanDisplacement& other)
					const;
	Point	operator()(const ImagePoint& p) const;
	Point	operator()(const Point& p) const;
};

/**
 * \brief Apply a euclidean displacement to an image
 */
template<typename Pixel>
class EuclideanDisplacementAdapter : public ConstImageAdapter<Pixel> {
	const ConstImageAdapter<Pixel>&	_image;
	EuclideanDisplacement	_displacement;
	Pixel   _defaultpixel;
public:
	EuclideanDisplacementAdapter(const ConstImageAdapter<Pixel>& image,
		const EuclideanDisplacement& displacement)
		: ConstImageAdapter<Pixel>(image.getSize()), _image(image),
		  _displacement(displacement) {
                if (std::numeric_limits<Pixel>::has_quiet_NaN) {
                        _defaultpixel = std::numeric_limits<Pixel>::quiet_NaN();
                } else {
                        _defaultpixel = Pixel(0);
                }
	}
	Pixel	pixel(int x, int y) const {
		Point	where = _displacement(Point(x, y));
		ImagePoint	w2(where.x(), where.y());
		if (_image.getSize().contains(w2)) {
			return _image.pixel(w2);
		}
		return _defaultpixel;
	}
};

/**
 * \brief Apply a euclidean displacement with interpolation to an image
 */
template<typename Pixel>
class InterpolatingEuclideanDisplacementAdapter
	: public ConstImageAdapter<Pixel> {
	const PixelInterpolationAdapter<Pixel> _image;
	EuclideanDisplacement	_displacement;
	Pixel   _defaultpixel;
public:
	InterpolatingEuclideanDisplacementAdapter(
		const ConstImageAdapter<Pixel>& image,
		const EuclideanDisplacement& displacement)
		: ConstImageAdapter<Pixel>(image.getSize()), _image(image),
		  _displacement(displacement) {
                if (std::numeric_limits<Pixel>::has_quiet_NaN) {
                        _defaultpixel = std::numeric_limits<Pixel>::quiet_NaN();
                } else {
                        _defaultpixel = Pixel(0);
                }
	}
	Pixel	pixel(int x, int y) const {
		Point	where = _displacement(ImagePoint(x, y));
		return _image.pixel(where);
	}
};

/**
 * \brief Interface class for functions defined on the group
 */
class EuclideanDisplacementFunction {
public:
	virtual double	operator()(const EuclideanDisplacement& g) const = 0;
};

template<typename Pixel>
class EuclideanDisplacementConvolve {
	const EuclideanDisplacementFunction&	_f;
	int	_angleresolution;
public:
	EuclideanDisplacementConvolve(const EuclideanDisplacementFunction& f,
		int angleresolution)
		: _f(f), _angleresolution(angleresolution) { }
	Image<Pixel>	*operator()(const ConstImageAdapter<Pixel>& image)
				const;
};

// specialization for double
template<>
Image<double>	*EuclideanDisplacementConvolve<double>::operator()(
			const ConstImageAdapter<double>& image) const;

} // namespace transform
} // namespace image
} // namespace astro

#endif /* _AstroGroup_h */
