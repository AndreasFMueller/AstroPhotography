/*
 * Transform.cpp -- image transform implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <cstdlib>
#include <AstroTransform.h>
#include <AstroFormat.h>
#include "TransformBuilder.h"

#ifdef HAVE_ACCELERATE_ACCELERATE_H
#include <Accelerate/Accelerate.h>
#else
#include <lapack.h>
#endif /* HAVE_ACCELERATE_ACCELERATE_H */

namespace astro {
namespace image {
namespace transform {

static const double	epsilon = 1e-10;

//////////////////////////////////////////////////////////////////////
// Translation operation
//////////////////////////////////////////////////////////////////////
#define	translate_typed(Pixel) 						\
{									\
	Image<Pixel>	*imageptr					\
		= dynamic_cast<Image<Pixel > *>(&*source);		\
	if (NULL != imageptr) {						\
		TranslationAdapter<Pixel >				\
			ta(*imageptr, translation);			\
		return ImagePtr(new Image<Pixel >(ta));			\
	}								\
}

ImagePtr	translate(ImagePtr source, const Point& translation) {
	translate_typed(unsigned char);
	translate_typed(unsigned short);
	translate_typed(unsigned int);
	translate_typed(unsigned long);
	translate_typed(float);
	translate_typed(double);
	translate_typed(RGB<unsigned char>);
	translate_typed(RGB<unsigned short>);
	translate_typed(RGB<unsigned int>);
	translate_typed(RGB<unsigned long>);
	translate_typed(RGB<float>);
	translate_typed(RGB<double>);
	throw std::runtime_error("cannot translate this image type");
}

//////////////////////////////////////////////////////////////////////
// Transform implementation
//////////////////////////////////////////////////////////////////////

void	Transform::identity() {
	a[0] = 1; a[1] = 0; a[2] = 0;
	a[3] = 0; a[4] = 1; a[5] = 0;
}

/**
 * \brief Default transform is the identity
 */
Transform::Transform() {
	identity();
}

/**
 * \brief Clone a transform
 */
Transform::Transform(const Transform& other) {
	for (size_t i = 0; i < 6; i++) {
		a[i] = other.a[i];
	}
}

/**
 * \brief Create an affine transform from angle translation and scale factor
 */
Transform::Transform(double angle, const Point& translation, double scale) {
	a[0] = scale * cos(angle); a[1] = -scale * sin(angle);
	a[3] = scale * sin(angle); a[4] =  scale * cos(angle);
	a[2] = translation.x();
	a[5] = translation.y();
}

/**
 * \brief Compute the inverse transformation
 */
Transform	Transform::inverse() const {
	Transform	result;
	// inverse of the matrix
	double	det = a[0] * a[4] - a[1] * a[3];
	result.a[0] =  a[4] / det;
	result.a[1] = -a[1] / det;
	result.a[3] = -a[3] / det;
	result.a[4] =  a[0] / det;
	// offset
	result.a[2] = -(result.a[0] * a[2] + result.a[1] * a[5]);
	result.a[5] = -(result.a[3] * a[2] + result.a[4] * a[5]);
	return result;
}

double	Transform::discrepancy(const ImageSize& size) const {
	double	m[4];

	Point	p1(0, 0);
	Point	p2(size.width(), 0);
	Point	p3(0, size.height());
	Point	p4(size.width(), size.height());

	m[0] = ((*this)(p1) - p1).abs();
	m[1] = ((*this)(p2) - p2).abs();
	m[2] = ((*this)(p3) - p3).abs();
	m[3] = ((*this)(p4) - p4).abs();

	return *std::max_element(m, m + 4);
}

/**
 * \brief Test whether this is a translation
 */
bool	Transform::isTranslation() const {
	if (fabs(a[0] - 1) > epsilon) { return false; }
	if (fabs(a[1] - 0) > epsilon) { return false; }
	if (fabs(a[3] - 0) > epsilon) { return false; }
	if (fabs(a[4] - 1) > epsilon) { return false; }
	return true;
}

bool	Transform::isIdentity() const {
	return isTranslation() && fixesOrigin();
}

bool	Transform::fixesOrigin() const {
	if (fabs(a[2]) > epsilon) { return false; }
	if (fabs(a[5]) > epsilon) { return false; }
	return true;
}

bool	Transform::isRotation() const {
	return fixesOrigin() && isIsometry();
}

bool	Transform::isHomothety() const {
	if (!fixesOrigin()) { return false; }
	if (fabs(a[0] - a[4]) > epsilon) { return false; }
	if (fabs(a[1]) > epsilon) { return false; }
	if (fabs(a[3]) > epsilon) { return false; }
	return true;
}

bool	Transform::isIsometry() const {
	// compute the product a*a', if this gives the identity,
	// the matrix of the transform is orthogonal, so it's an
	// isometry
	if (fabs((a[0] * a[0] + a[1] * a[1]) - 1) > epsilon) { return false; }
	if (fabs((a[0] * a[3] + a[1] * a[4]) - 0) > epsilon) { return false; }
	if (fabs((a[3] * a[3] + a[3] * a[4]) - 1) > epsilon) { return false; }
	return true;
}

bool	Transform::isAreaPreserving() const {
	double	det = a[0] * a[4] - a[1] * a[3];
	if (fabs(fabs(det) - 1) > epsilon) {
		return false;
	}
	return true;
}

bool	Transform::isAnglePreserving() const {
	if (fabs(a[0] * a[3] + a[1] * a[4]) > epsilon) { return false; }
	if (fabs((a[0] * a[0] + a[1] * a[1]) - (a[3] * a[3] + a[4] * a[4]))
		> epsilon) { return false; }
	return true;
}

double	Transform::skew() const {
	double	d[3];
	d[0] = a[0] * a[0] + a[3] * a[3];
	d[1] = a[0] * a[1] + a[3] * a[4];
	d[2] = a[1] * a[1] + a[4] * a[4];
	return hypot(d[1], d[0]/d[2] - 1);
}

bool	Transform::isAspectPreserving() const {
	if (skew() > epsilon) { return false; }
	return true;
}

bool	Transform::operator==(const Transform& other) const {
	for (size_t i = 0; i < 6; i++) {
		if (fabs(a[i] - other.a[i]) > epsilon) { return false; }
	}
	return true;
}

bool	Transform::operator!=(const Transform& other) const {
	return !operator==(other);
}

/**
 * \brief Extract the translation component
 */
Point	Transform::getTranslation() const {
	return Point(a[2], a[5]);
}

/*
 * \brief Composition operator
 */
Transform	Transform::operator*(const Transform& other) const {
	Transform	result;
	// matrix product
	result.a[0] = a[0] * other.a[0] + a[1] * other.a[3];
	result.a[1] = a[0] * other.a[1] + a[1] * other.a[4];
	result.a[3] = a[3] * other.a[0] + a[4] * other.a[3];
	result.a[4] = a[3] * other.a[1] + a[4] * other.a[4];
	// operation 
	Point	composed = this->operator()(other.getTranslation());
	result.a[2] = composed.x();
	result.a[5] = composed.y();
	return result;
}

Transform	Transform::operator+(const Point& translation) const {
	Transform	result;
	for (int i = 0; i < 6; i++) {
		result.a[i] = a[i];
	}
	result.a[2] += translation.x();
	result.a[5] += translation.y();
	return result;
}

Transform	Transform::operator+(const ImagePoint& translation) const {
	return operator+(Point(translation));
}

double	Transform::operator[](int i) const {
	if ((i < 0) || (i > 5)) {
		throw std::range_error("out of range");
	}
	return a[i];
}

double&	Transform::operator[](int i) {
	if ((i < 0) || (i > 5)) {
		throw std::range_error("out of range");
	}
	return a[i];
}

Point	Transform::operator()(const Point& point) const {
	return Point(
		a[0] * point.x() + a[1] * point.y() + a[2],
		a[3] * point.x() + a[4] * point.y() + a[5]
	);
}

/**
 * \brief Display version of string transform
 */
std::ostream&	operator<<(std::ostream& out, const Transform& transform) {
	out << transform.toString() << std::endl;
	return out;
}

std::string	Transform::toString() const {
	return stringprintf("[ %f, %f, %f; %f, %f, %f ]",
		a[0], a[1], a[2], a[3], a[4], a[5]);
}

//////////////////////////////////////////////////////////////////////
// Transform implementation
//////////////////////////////////////////////////////////////////////
#define	transform_typed(Pixel)						\
{									\
	Image<Pixel >	*imageptr					\
		= dynamic_cast<Image<Pixel > *>(&*image);		\
	if (NULL != imageptr) {						\
		TransformAdapter<Pixel > ta(*imageptr, transform);	\
		return ImagePtr(new Image<Pixel >(ta));			\
	}								\
}

ImagePtr	transform(ImagePtr image, const Transform& transform) {
	transform_typed(unsigned char);
	transform_typed(unsigned short);
	transform_typed(unsigned int);
	transform_typed(unsigned long);
	transform_typed(float);
	transform_typed(double);
	transform_typed(RGB<unsigned char>);
	transform_typed(RGB<unsigned short>);
	transform_typed(RGB<unsigned int>);
	transform_typed(RGB<unsigned long>);
	transform_typed(RGB<float>);
	transform_typed(RGB<double>);
	throw std::runtime_error("cannot transform image of this pixel type");
}

} // namespace transform
} // namespace image
} // namespace astro
