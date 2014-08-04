/*
 * AstroProjection.h -- Projection of images
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroProjection_h
#define _AstroProjection_h

#include <AstroTypes.h>
#include <AstroImage.h>

namespace astro {
namespace image {
namespace project {

/**
 * \brief A projection
 *
 * Projections are affine transformations composed with a radius dependent
 * homothety.
 */
class Projection {
	float	a[6];	// 
	float	b[2];	// 
	float	w(float r) const;
	float	wi(float r) const;
public:
	Projection();
	Projection	inverse() const;
	Point	operator()(const Point& p) const;
	Point	operator()(const ImagePoint& p) const;
};

#if 0
/**
 * \brief Apply a projection to an image
 */
class ProjectionAdapter : public ConstImageAdapter<float> {
	const ConstImageAdapter<float>& image;
	Projection	projection;
public:
	ProjectionAdapter(const ConstImageAdapter<float>& image,
		const Projection& projection);
	virtual const float	pixel(unsigned int x, unsigned int y) const;
};
#endif

} // namespace project
} // namespace image
} // namespace astro

#endif /* _AstroProjection_h */
