/*
 * BacklashAnalysis.cpp -- implementation of the backlash anaylsis
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <Backlash.h>
#include <AstroDebug.h>
#include "LinearRegression.h"

#ifdef HAVE_ACCELERATE_ACCELERATE_H
#include <Accelerate/Accelerate.h>
#else
#include <lapack.h>
#endif /* HAVE_ACCELERATE_ACCELERATE_H */


namespace astro {
namespace guiding {

static inline double	sqr(double x) { return x * x; }

/**
 * \brief Find the drift
 *
 * \param points	the data points
 * \param r		use the vector from this structure
 */
double	BacklashAnalysis::drift(const std::vector<BacklashPoint>& points,
		const BacklashResult& r) const {
	std::vector<std::pair<double, double> >	data[4];
	std::vector<BacklashPoint>::const_iterator	i = points.begin();
	int	j = 0;
	while (i != points.end()) {
		double	X = i->time;
		double	Y = i->xoffset * r.x + i->yoffset * r.y;
		data[j % 4].push_back(std::make_pair(X, Y));
		i++;
		j++;
	}
	double	a = 0;
	for (j = 0; j < 4; j++) {
		linear::LinearRegression	lr(data[j]);
		a += lr.a();
	}
	return a/4;
}

/**
 * \brief Skip a suitable number of points
 *
 * If _lastpoints is 0, all points are used
 */
std::vector<BacklashPoint>::const_iterator	BacklashAnalysis::begin(const std::vector<BacklashPoint>& points) const {
	std::vector<BacklashPoint>::const_iterator	i = points.begin();
	if (_lastpoints == 0) {
		return i;
	}
	int	n = points.size();
	int	counter = 0;
	while (counter < (n - _lastpoints - 4)) {
		i += 4;
		counter += 4;
	}
	return i;
}

/**
 * \brief Perform an Analysis of the backlash data
 *
 * \param points	data points to use to analyze 
 */
BacklashResult	BacklashAnalysis::operator()(
	const std::vector<BacklashPoint>& points) {
	BacklashResult	r;
	r.direction = _direction;
	r.interval = _interval;
	r.lastpoints = _lastpoints;
	int	n = 0;

	// compute the covariance matrix of the points
	double	C[4] = { 0, 0, 0, 0 };
	double	M[2] = { 0, 0 };
	std::vector<BacklashPoint>::const_iterator	start = begin(points);
	std::vector<BacklashPoint>::const_iterator	i = start;
	while (i != points.end()) {
		double	x = i->xoffset;
		double	y = i->yoffset;
		M[0] += x;
		M[1] += y;
		C[0] += x * x;
		C[1] += x * y;
		C[2] += y * x;
		C[3] += y * y;
		i++;
		n++;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "analyzing %d points", n);
	for (int k = 0; k < 2; k++) {
		M[k] /= n;
	}
	for (int k = 0; k < 4; k++) {
		C[k] /= n;
	}
	C[0] -= M[0] * M[0];
	C[1] -= M[1] * M[0];
	C[2] -= M[0] * M[1];
	C[3] -= M[1] * M[1];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "covariance matrix:");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "[ %10.2f %10.2f ]", C[0], C[1]);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "[ %10.2f %10.2f ]", C[2], C[3]);

	// compute the eigenvectors of the covariance matrix, we need the
	// one for the larger eigenvalue, this is the x,y-direction
	double	trace = C[0] + C[3];
	double	det = C[0] * C[3] - C[1] * C[2];
	double	lambda1 = trace/2 + sqrt(sqr(trace)/4 - det);
	double	lambda2 = trace/2 - sqrt(sqr(trace)/4 - det);
	r.x = lambda1 - C[3];
	r.y = C[2];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "lambda1 = %f, lambda2 = %f",
		lambda1, lambda2);

	// normalize the the direction vector
	double	l = hypot(r.x, r.y);
	r.x /= l;
	r.y /= l;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "eigenvector: %f, %f", r.x, r.y);

	// find the drift
	r.drift = drift(points, r);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "drift = %f", r.drift);

	// project the direction to find the X-array
	double	X[n];
	i = start;
	int	k = 0;
	l = 0;
	double	l2 = 0;
	while (i != points.end()) {
		X[k] = i->xoffset * r.x + i->yoffset * r.y;
		double	L = i->xoffset * r.y - i->yoffset * r.x;
		l += L;
		l2 += sqr(L);
		i++;
		k++;
	}

	// compute lateral variance
	r.lateral = sqrt(((l2 / n) - sqr(l / n)) * (n / (n - 1)));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "lateral variance: %f", r.lateral);

	// compute the matrix
	int	m = 5;
	double	A[n * m];
	double	b[n];
	i = start;
	int	K[4] = {0, 0, 0, 0};
	int	s = 0;
	while (points.end() != i) {
		// fill in the right hand side vector
		A[s        ] =  K[0];
		A[s + 1 * n] =  K[1];
		A[s + 2 * n] = -K[2];
		A[s + 3 * n] = -K[3];
		A[s + 4 * n] =   1;
		b[s] = X[s] - r.drift * i->time;
		K[s % 4]++;
		i++;
		s++;
	}
	for (s = 0; s < n; s++) {
		for (int t = s + 1; t < n; t++) {
			A[t        ] -= A[s        ];
			A[t + 1 * n] -= A[s + 1 * n];
			A[t + 2 * n] -= A[s + 2 * n];
			A[t + 3 * n] -= A[s + 3 * n];
			A[t + 4 * n] -= A[s + 4 * n];
			b[t] -= b[s];
		}
	}
	for (s = 0; s < n; s++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"%4.0f %4.0f %4.0f %4.0f  %8.4f   %8.4f",
			A[s        ], A[s + 1 * n], A[s + 2 * n],
			A[s + 3 * n], A[s + 4 * n],
			b[s]);
	}

	// solve the least squares problem
	// prepare to solve the system using LAPACK (dgels_)
	char	trans = 'N';
	int	nrhs = 1;
	int	lda = n;
	int	ldb = n;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "lda = %d, ldb = %d", lda, ldb);
	int	lwork = -1;
	int	info = 0;

        // determine work area size
	double  x;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "solving a %d x %d system", n, m);
	dgels_(&trans, &n ,&m, &nrhs, A, &lda, b, &ldb, &x, &lwork, &info);
	if (info != 0) {
		std::string     msg = stringprintf("dgels cannot determine "
			"work area size: %d", info);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	lwork = x;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "need work area of size %d", lwork);

	//  allocate work array
	double  work[lwork];
	dgels_(&trans, &n ,&m, &nrhs, A, &lda, b, &ldb, work, &lwork, &info);
	if (info != 0) {
		std::string     msg = stringprintf("dgels cannot solve "
			"equations: %d", info);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"solution: %6.2f %6.2f %6.2f %6.2f %6.2f",
		b[0], b[1], b[2], b[3], b[4]);

	// get the results from the b vector
	r.f = b[0];
	r.forward = b[1];
	r.b = b[2];
	r.backward = b[3];
	r.offset = b[4];

	// compute the offset variance
	l = 0; l2 = 0;
	i = start;
	for (int t = 0; t < 4; t++) { K[t] = 0; }
	s = 0;
	while (points.end() != i) {
		b[s] = X[s];
		double	delta = 0;
		delta = X[s] - r(K, *i);
		K[s % 4]++;
		l += delta;
		l2 += sqr(delta);
		s++;
		i++;
	}
	r.longitudinal = ((l2 / n) - sqr(l / n)) * (n / (n - 1));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "longitudinal variance: %f",
		r.longitudinal);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "analysis result: %s",
		r.toString().c_str());

	// done
	return r;
}

} // namespace guiding
} // namespace astro
