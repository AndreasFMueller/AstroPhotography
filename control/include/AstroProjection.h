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
	Point	operator()(const Point& p) const;
	double	operator[](int i) const;
	double&	operator[](int i);
};

/**
 * \brief Apply a projection to an image
 */
template<typename Pixel>
class ProjectionAdapter : public ConstImageAdapter<Pixel> {
	const PixelInterpolationAdapter<Pixel> image;
	Projection	projection;
	Point	center;
	Point	targetcenter;
public:
	ProjectionAdapter(const ImageSize targetsize,
		const ConstImageAdapter<Pixel>& _image,
		const Projection& _projection)
		: ConstImageAdapter<Pixel>(targetsize), image(_image),
		  projection(_projection) {
		center = ConstImageAdapter<Pixel>::getSize().center();
		targetcenter = targetsize.center();
	}
	virtual const Pixel	pixel(unsigned int x, unsigned int y) const;
};

template<typename Pixel>
const Pixel	ProjectionAdapter<Pixel>::pixel(unsigned int x, unsigned int y) const {
	Point	p(x - center.x(), y - center.y());
	return image.pixel(projection(p) + targetcenter);
}

/**
 * \brief Correct a projection from a list of Residuals
 */
class ProjectionCorrector {
	ImageSize	size;
	Projection	projection;
	const std::vector<Residual>&	residuals;
public:
	ProjectionCorrector(const ImageSize& _size,
		const Projection& _projection,
		const std::vector<Residual>& _residuals)
		: size(_size), projection(_projection), residuals(_residuals) {
	}
	Projection	corrected() const;
};

} // namespace project
} // namespace image
} // namespace astro

#endif /* _AstroProjection_h */
