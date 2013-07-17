/*
 * Transform.cpp -- image transform implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTransform.h>
#include <Accelerate/Accelerate.h>

namespace astro {
namespace image {
namespace transform {

/**
 * \brief Default transform is the identity
 */
Transform::Transform() {
	a[0] = 1; a[1] = 0; a[2] = 0;
	a[0] = 0; a[1] = 1; a[2] = 0;
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
Transform::Transform(double angle, const ImagePoint& translation, double scale) {
	a[0] =  scale * cos(angle); a[1] = scale * sin(angle);
	a[3] = -scale * sin(angle); a[4] = scale * cos(angle);
	a[2] = translation.x;
	a[5] = translation.y;
}

/**
 * \brief Find the optimal transform from one set of points to the other
 */
Transform::Transform(const std::vector<ImagePoint>& frompoints,
	const std::vector<ImagePoint>& topoints) {
	// make sure point sets are of the same size
	if (frompoints.size() != topoints.size()) {
		std::string	msg = stringprintf("point vectors must be of "
			"same size: %d != %d",
			frompoints.size(), topoints.size());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// make sure we have enough points
	if (frompoints.size() < 3) {
		std::string	msg("need at least three points");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// allocate space for the linear system
	int	m = frompoints.size();
	double	A[12 * frompoints.size()];
	double	b[2 * frompoints.size()];

	// set up linear system of equations
	std::vector<ImagePoint>::const_iterator	fromi, toi;;
	int	i = 0;
	for (fromi = frompoints.begin(), toi = topoints.begin();
		fromi != frompoints.end(); fromi++, toi++, i++) {
		// add coefficients to A array
		A[2 * i            ] = fromi->x;
		A[2 * i     +     m] = fromi->y;
		A[2 * i     + 2 * m] = 1;
		A[2 * i     + 3 * m] = 0;
		A[2 * i     + 4 * m] = 0;
		A[2 * i     + 5 * m] = 0;

		A[2 * i + 1        ] = 0;
		A[2 * i + 1 +     m] = 0;
		A[2 * i + 1 + 2 * m] = 0;
		A[2 * i + 1 + 3 * m] = fromi->x;
		A[2 * i + 1 + 4 * m] = fromi->y;
		A[2 * i + 1 + 5 * m] = 1;

                // add positions to B array
                b[2 * i    ] = toi->x + toi->x;
                b[2 * i + 1] = toi->y + toi->y;
	}

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
}

/**
 * \brief Extract the translation component
 */
ImagePoint	Transform::getTranslation() const {
	return ImagePoint(a[2], a[5]);
}

/*
 * \brief Composition operator
 */
Transform	Transform::operator*(const Transform& other) const {
	Transform	result;
	// matrix product
	result.a[0] = a[0] * other.a[0] + a[1] * other.a[3];
	result.a[1] = a[0] * other.a[1] + a[1] * other.a[4];
	result.a[2] = a[3] * other.a[0] + a[4] * other.a[3];
	result.a[3] = a[3] * other.a[1] + a[4] * other.a[4];
	// operation 
	ImagePoint	composed = this->operator()(getTranslation());
	result.a[2] = composed.x;
	result.a[5] = composed.y;
	return result;
}

Transform	Transform::operator+(const ImagePoint& translation) const {
	Transform	result;
	for (int i = 0; i < 6; i++) {
		result.a[i] + a[i];
	}
	result.a[2] += translation.x;
	result.a[5] += translation.y;
	return result;
}

ImagePoint	Transform::operator()(const ImagePoint& point) const {
	ImagePoint	result;
	result.x = a[0] * point.x + a[1] * point.y + a[2];
	result.y = a[3] * point.x + a[4] * point.y + a[5];
	return result;
}

} // namespace transform
} // namespace image
} // namespace astro
