/*
 * AstroProjection.h -- Projection of images
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroProjection_h
#define _AstroProjection_h

#include <AstroTypes.h>
#include <AstroImage.h>
#include <AstroTransform.h>

namespace astro {
namespace image {
namespace transform {

/**
 * \brief A projection
 *
 * Projections are affine transformations composed with a radius dependent
 * homothety.
 */
class Projection : public Transform {
	double	b[2];
	double	w(double r) const;
public:
	Projection();
	Projection(double angle, const Point& translation,
                double scalefactor = 1);
	virtual Point	operator()(const Point& p) const;
	virtual double	operator[](int i) const;
	virtual double&	operator[](int i);
	virtual std::string	toString() const;
};

/**
 * \brief A projection with centers different from (0,0)
 */
class CenteredProjection : public Projection {
	Point	targetcenter;
	Point	center;
public:
	CenteredProjection(const Point _targetcenter, const Point _center,
		Projection _projection) : Projection(_projection),
			targetcenter(_targetcenter), center(_center) {
	}
	virtual Point	operator()(const Point& p) const;
	virtual Point	operator()(unsigned int x, unsigned int y) const;
};

/**
 * \brief Apply a projection to an image
 */
template<typename Pixel>
class ProjectionAdapter : public ConstImageAdapter<Pixel> {
	const PixelInterpolationAdapter<Pixel> image;
	CenteredProjection	centeredprojection;
public:
	ProjectionAdapter(const ImageSize targetsize,
		const ConstImageAdapter<Pixel>& _image,
		const Projection& _projection)
		: ConstImageAdapter<Pixel>(targetsize), image(_image),
		  centeredprojection(targetsize.center(),
			ConstImageAdapter<Pixel>::getSize().center(),
			_projection) {
	}
	virtual Pixel	pixel(unsigned int x, unsigned int y) const;
};

template<typename Pixel>
Pixel	ProjectionAdapter<Pixel>::pixel(unsigned int x, unsigned int y) const {
	return image.pixel(centeredprojection(x, y));
}

/**
 * \brief Correct a projection from a list of Residuals
 */
class ProjectionCorrector {
	CenteredProjection	centeredprojection;
public:
	ProjectionCorrector(const ImageSize& _targetsize,
		const ImageSize& _size, const Projection& _projection)
		: centeredprojection(_targetsize.center(), _size.center(),
			_projection) {
	}
	Projection	corrected(const std::vector<Residual>& residuals) const;
};

} // namespace project
} // namespace image
} // namespace astro

#endif /* _AstroProjection_h */
