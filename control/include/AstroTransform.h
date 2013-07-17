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
 * \brief Abstraction of an affine transform
 */

class Transform {
	double	a[6];
public:
	// constructors
	Transform();
	Transform(const Transform& other);
	Transform(double angle, const ImagePoint& translation,
		double scalefactor = 1);
	Transform(const std::vector<astro::image::ImagePoint>& frompoints,
		const std::vector<astro::image::ImagePoint>& topoints);

	// data access
	ImagePoint	getTranslation() const;

	// operations
	Transform	operator*(const Transform& other) const;
	Transform	operator+(const astro::image::ImagePoint& translation) const;

	// operating on points and images
	ImagePoint	operator()(const astro::image::ImagePoint& point) const;
};

} // namespace transform
} // namespace image
} // namespace astro

#endif /* _AstroTransform_h */
