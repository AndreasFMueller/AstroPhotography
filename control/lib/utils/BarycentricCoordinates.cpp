/*
 * BarycentricCoordinates.cpp -- implementation of barycentric coordinates
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCoordinates.h>
#include <AstroFormat.h>

#ifdef HAVE_ACCELERATE_ACCELERATE_H
#include <Accelerate/Accelerate.h>
#else
#include <lapack.h>
#endif /* HAVE_ACCELERATE_ACCELERATE_H */

namespace astro {

//////////////////////////////////////////////////////////////////////
// implementation of the BarycentricPoint class
//////////////////////////////////////////////////////////////////////

BarycentricPoint::BarycentricPoint(double _w1, double _w2, double _w3)
	: Point(_w1, _w2) {
	double	d = 1 - _w1 - _w2 - _w3;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "d = %f", d);
	if (fabs(d) > 1e-10) {
		std::string	cause = stringprintf("%f + %f + %f != 1",
			_w1, _w2, _w3);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw std::runtime_error(cause);
	}
}

std::string	BarycentricPoint::toString() const {
	return stringprintf("(w1=%.3f,w2=%.3f,w3=%.3f)", w1(), w2(), w3());
}

bool	BarycentricPoint::inside() const {
	return (w1() >= 0) && (w2() >= 0) && (w3() >= 0);
}

//////////////////////////////////////////////////////////////////////
// Implementation of the BarycentricCoordinates class
//////////////////////////////////////////////////////////////////////

/**
 * \brief Construct a barycentric coordinate system
 *
 * The constructor computes the Inverse of the matrix with the point
 * coordinates as rows
 */
BarycentricCoordinates::BarycentricCoordinates(const Point& p1,
	const Point& p2, const Point& p3)
	: _p1(p1), _p2(p2), _p3(p3) {
	// we need 18 doubles to compute the 

	int	n = 3;
	int	nrhs = 3;
	double	a[9];		// coefficients
	int	lda = 3;
	int	ipiv[3];
	int	ldb = 3;
	int	info;

	// initialize the arrays
	a[0] = p1.x();	a[3] = p2.x();	a[6] = p3.x();
	a[1] = p1.y();	a[4] = p2.y();	a[7] = p3.y();
	a[2] = 1;	a[5] = 1;	a[8] = 1;	

	// we want the result in the b array
	b[0] = 1;	b[3] = 0;	b[6] = 0;
	b[1] = 0;	b[4] = 1;	b[7] = 0;
	b[2] = 0;	b[5] = 0;	b[8] = 1;

	dgesv_(&n, &nrhs, a, &lda, ipiv, b, &ldb, &info);
	if (0 != info) {
		std::string	cause = stringprintf("dgesv fails: info = %d",
			info);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw std::runtime_error(cause);
	}
}

BarycentricPoint	BarycentricCoordinates::operator()(const Point& point) const {
	double	_w1 = point.x() * b[0] + point.y() * b[3] + b[6];
	double	_w2 = point.x() * b[1] + point.y() * b[4] + b[7];
	double	_w3 = point.x() * b[2] + point.y() * b[5] + b[8];
	return BarycentricPoint(_w1, _w2, _w3);
}

Point	BarycentricCoordinates::operator()(const BarycentricPoint& b) const {
	return	_p1 * b.w1() + _p2 * b.w2() + _p3 * b.w3();
}

std::string	BarycentricCoordinates::toString() const {
	return	stringprintf("[ %8.3f, %8.3f, %8.3f;\n"
			     "  %8.3f, %8.3f, %8.3f;\n"
			     "  %8.3f, %8.3f, %8.3f ]",
				b[0], b[3], b[6],
				b[1], b[4], b[7],
				b[2], b[5], b[8]);
}

bool	BarycentricCoordinates::inside(const Point& point) const {
	return this->operator()(point).inside();
}

} // namespace astro
