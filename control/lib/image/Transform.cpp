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

#if 0
/**
 * \brief Extract the average translation from a number of residuals
 */
void	Transform::translation(const std::vector<Residual>& residuals) {
	if (residuals.size() == 0) {
		throw std::runtime_error("need at least one residual to "
			"extract translation");
	}
	identity();

	// compute the average of all translations
	Point	sum;
	for (auto r = residuals.begin(); r != residuals.end(); r++) {
		sum = sum + r->offset();
	}
	sum = (1. / residuals.size()) * sum;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "average translation: %s",
		std::string(sum).c_str());

	// the average translation becomes the constant part of the transform
	a[2] = sum.x();
	a[5] = sum.y();
}
#endif

#if 0
/**
 * \brief common method to solve least squares equations
 */
void	Transform::build(const std::vector<Point>& from,
		const std::vector<Point>& to,
		const std::vector<double>& weights) {
	if (from.size() != to.size()) {
		std::string	msg = stringprintf("point vector size mismatch:"
			" %d != %d", from.size(), to.size());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	bool	ignore_weights = (from.size() != weights.size());

	// allocate space for the linear system
	int	m = 2 * from.size();
	double	A[6 * m];
	double	b[m];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "A size: %d, b size: %d", 6 * m, m);

	// set up linear system of equations
	int	i = 0;
	auto	fromptr = from.begin();
	auto	toptr = to.begin();
	auto	weightptr = weights.begin();
	while (fromptr != from.end()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s ~ %s, delta = %s",
			fromptr->toString().c_str(),
			toptr->toString().c_str(),
			(*toptr - *fromptr).toString().c_str());
		double	weight = (ignore_weights) ? 1. : *weightptr;
		// add coefficients to A array
		A[i        ] = fromptr->x() * weight;
		A[i +     m] = fromptr->y() * weight;
		A[i + 2 * m] = 1 * weight;
		A[i + 3 * m] = 0;
		A[i + 4 * m] = 0;
		A[i + 5 * m] = 0;

                b[i] = toptr->x() * weight;

		i++;

		A[i        ] = 0;
		A[i +     m] = 0;
		A[i + 2 * m] = 0;
		A[i + 3 * m] = fromptr->x() * weight;
		A[i + 4 * m] = fromptr->y() * weight;
		A[i + 5 * m] = 1 * weight;

                b[i] = toptr->y() * weight;

		i++;

		fromptr++;
		toptr++;
		if (!ignore_weights) { weightptr++; }
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "number of equations: %d", i);

	// solve the linear system
	char	trans = 'N';
	int	n = 6;
	int	nrhs = 1;
	int	lda = m;
	int	ldb = m;
	int	lwork = -1;
	int	info = 0;

	// first call to dgels is set up to determine the needed size of the
	// work array.
	double	x;
	dgels_(&trans, &m, &n, &nrhs, A, &lda, b, &ldb, &x, &lwork, &info);
	if (info != 0) {
		std::string	msg = stringprintf("dgels cannot determine "
			"work area size: %d", info);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	lwork = x;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "need work area of size %d", lwork);

	// with the correct work array in place, the next call solves the
	// equations
	double	work[lwork];
	dgels_(&trans, &m, &n, &nrhs, A, &lda, b, &ldb, work, &lwork, &info);
	if (info != 0) {
		std::string	msg = stringprintf("dgels cannot solve "
			"equations: %d", info);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// copy result vector
	for (int i = 0; i < 6; i++) {
		a[i] = b[i];
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "transformation found: %s",
		this->toString().c_str());

	// compute the residual
	double	residual = 0.;
	fromptr = from.begin();
	toptr = to.begin();
	i = 0;
	while (fromptr != from.end()) {
		double	X, Y;
		X = a[0] * fromptr->x() + a[1] * fromptr->y() + a[2];
		Y = a[3] * fromptr->x() + a[4] * fromptr->y() + a[5];
		double	delta = hypot(X - toptr->x(), Y - toptr->y());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "residual[%d] = %f",
			i++, delta);
		residual += delta;
		fromptr++;
		toptr++;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "residual = %f", residual);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "average residual %f",
		residual / from.size());
} 
/**
 * \brief common method to solve least squares equations
 */
void	Transform::build(const std::vector<Point>& from,
		const std::vector<Point>& to,
		const std::vector<double>& weights) {
	AffineTransformBuilder	atb;
	*this = atb.build(from, to, weights);
}
#endif

#if 0
Transform::Transform(const std::vector<Residual>& residuals) {
	// make sure we have enough points
	if (residuals.size() < 3) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "not enough data for full "
			"transform, extracting translation only");
		translation(residuals);
		return;
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "determine best transformation between "
		"two sets of %d points", residuals.size());
	std::vector<Point>	from;
	std::vector<Point>	to;
	std::vector<double>	weights;
	for (auto ptr = residuals.begin(); ptr != residuals.end(); ptr++) {
		Point	f = ptr->from();
		from.push_back(f);
		to.push_back(f + ptr->offset());
		weights.push_back(ptr->weight());
	}
	build(from, to, weights);
}

Transform::Transform(const std::vector<Point>& from,
	const std::vector<Point>& to) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "build transform from %d matching points", from.size());
	std::vector<double>	weights(from.size(), 1.);
	build(from, to, weights);
}

Transform::Transform(const std::vector<Point>& from,
	const std::vector<Point>& to, const std::vector<double>& weights) {
	build(from, to, weights);
}
#endif

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
