/*
 * FocusCompute.cpp -- compute the optimal focus position
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <FocusCompute.h>
#include <AstroDebug.h>
#include <lapack.h>
#include <AstroFormat.h>
#include <math.h>

namespace astro {
namespace focusing {

FocusCompute::FocusCompute() {
}


/**
 * \brief Compute the solution
 */
std::pair<double, double>	FocusCompute::solve(double *positions,
					double *values) const {
	// allocate memory for dgels_
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d x 2 system of equations", size());
	int	m = size();
	int	n = 2;
	char	trans = 'N';
	int	nrhs = 1;
	int	lda = size();
	int	ldb = size();
	int	lwork = -1;
	int	info = 0;	
	double	a[2 * size()];
	double	b[size()];
	for (int i = 0; i < size(); i++) {
		a[i] = positions[i];
		a[i + m] = 1.;
		b[i] = values[i];
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%f * a + b = %g",
			positions[i], values[i]);
	}

	// first find out how large the work area must be
	double	x;
	dgels_(&trans, &m, &n, &nrhs, a, &lda, b, &ldb, &x, &lwork, &info);
	if (info != 0) {
		std::string	msg = stringprintf("dgels_ cannot determine "
					"work size: %d", info);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	lwork = x;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "work area size: %d", lwork);

	// create a work area and run the method again
	double	work[lwork];
	dgels_(&trans, &m, &n, &nrhs, a, &lda, b, &ldb, work, &lwork, &info);
	if (info != 0) {
		std::string	msg = stringprintf("dgels_ cannot solve "
					"equations: %d", info);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "b[0] = %g, b[1] = %g", b[0], b[1]);

	// compute the optimal position
	double	position = -b[1] / b[0];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tentative position: %f", position);

	// compute the error
	double	sum = 0;
	for (int i = 0; i < size(); i++) {
		double	d = b[0] * positions[i] + b[1] - values[i];
		sum += d * d;
	}

	// compute the result
	std::pair<double, double>	result;
	result.first = position;
	result.second = sqrt(sum);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "position: %f, error: %g",
		result.first, result.second);
	return result;
}

/**
 * \brief find the best focus position
 */
double	FocusCompute::focus() const {
	// copy the data into arrays
	double	positions[size()];
	double	values[size()];
	std::map<unsigned short, double>::const_iterator	i;
	int	offset = 0;
	for (i = begin(); i != end(); i++) {
		positions[offset] = i->first;
		values[offset] = i->second;
		offset++;
	}

	// unconditionally change the sign of the first entry
	double	errors[size()];

	// change sign of one value after the other
	for (int j = 0; j < size(); j++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "trying position %d", j);
		values[j] = -values[j];
		std::pair<double, double>	v = solve(positions, values);
		errors[j] = v.second;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "pos = %f, err = %f",
			v.first, v.second);
	}

	// find the minimum
	double	m = 0;
	for (int j = 0; j < size(); j++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "error[%d] = %f", j, errors[j]);
		if (m < errors[j]) {
			m = errors[j];
		}
	}
	int	jmin = -1;
	for (int j = 0; j < size() - 1; j++) {
		if (m > errors[j]) {
			jmin = j;
			m = errors[j];
		}
	}

	// if j is negative, then we have no solution
	if (jmin < 0) {
		throw std::runtime_error("no solution found");
	}

	// compute for position found
	for (int j = 0; j < size(); j++) {
		if (j <= jmin) {
			values[j] = -fabs(values[j]);
		} else {
			values[j] = fabs(values[j]);
		}
	}
	std::pair<double, double>	v = solve(positions, values);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "suggested position: %f", v.first);
	return v.first;
}

} // namespace focusing
} // namespace astro
