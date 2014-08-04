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
namespace project {

/**
 * \brief A projection
 *
 * Projections are affine transformations composed with a radius dependent
 * homothety.
 */
class Projection : public astro::image::transform::Transform {
	float	b[2];
	float	w(float r) const;
public:
	Projection();
	Point	operator()(const Point& p) const;
};

/**
 * \brief Apply a projection to an image
 */
template<typename Pixel>
class ProjectionAdapter : public ConstImageAdapter<Pixel> {
	const astro::image::transform::PixelInterpolationAdapter<Pixel> image;
	Projection	projection;
	Point	center;
	Point	targetcenter;
public:
	ProjectionAdapter(const ImageSize targetsize,
		const ConstImageAdapter<float>& _image,
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
	return image(projection(p) + targetcenter);
}

} // namespace project
} // namespace image
} // namespace astro

#endif /* _AstroProjection_h */
