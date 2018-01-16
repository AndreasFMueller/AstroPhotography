/*
 * Function.cpp -- algorithms to extract a background gradient from an image
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroBackground.h>
#include <AstroFormat.h>
#include <AstroFilter.h>
#include <AstroUtils.h>
#include <cmath>
#include <set>
#include <sstream>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_ACCELERATE_ACCELERATE_H
#include <Accelerate/Accelerate.h>
#else
#include <lapack.h>
#endif /* HAVE_ACCELERATE_ACCELERATE_H */

using namespace astro::image;
using namespace astro::image::filter;

namespace astro {
namespace adapter {

//////////////////////////////////////////////////////////////////////
// FunctionBase implementation
//////////////////////////////////////////////////////////////////////

FunctionBase::FunctionBase(const FunctionBase& other) :
	_symmetric(other.symmetric()), _center(other.center()) {
	_gradient = other.gradient();
	_scalefactor = other.scalefactor();
}

double	FunctionBase::evaluate(const ImagePoint& point) const {
	return evaluate(Point(point.x(), point.y()));
}

double	FunctionBase::evaluate(int x, int y) const {
	return evaluate(Point(x, y));
}

double	FunctionBase::operator()(const Point& point) const {
	return evaluate(point);
}

double	FunctionBase::operator()(const ImagePoint& point) const {
	return evaluate(point);
}

double	FunctionBase::operator()(int x, int y) const {
	return evaluate(x, y);
}

std::string	FunctionBase::toString() const {
	return stringprintf("[gradient=%s,symmetric=%s,scalefactor=%.3f]",
		(gradient()) ? "YES" : "NO", (symmetric()) ? "YES" : "NO",
		scalefactor());
}

//////////////////////////////////////////////////////////////////////
// LinearFunction implementation
//////////////////////////////////////////////////////////////////////

LinearFunction::LinearFunction(const ImagePoint& point, bool symmetric)
	: FunctionBase(point, symmetric) {
	for (int i = 0; i < 3; i++) {
		a[i] = 0;
	}
}

LinearFunction::LinearFunction(const LinearFunction& other) :
	FunctionBase(other) {
	for (int i = 0; i < 3; i++) {
		a[i] = other.a[i];
	}
}

double  LinearFunction::evaluate(const Point& point) const {
	double	value = a[2];
	if (gradient() && (!symmetric())) {
		double	deltax = point.x() - center().x();
		double	deltay = point.y() - center().y();
		value += (deltax * a[0] + deltay * a[1]);
	}
	return scalefactor() * value;
}

inline static double	sqr(const double& x) {
	return x * x;
}

double	LinearFunction::norm() const {
	double	result = 0;
	result += sqr(center().x() * a[0]);
	result += sqr(center().y() * a[1]);
	result += sqr(a[2]);
	return sqrt(result);
}

LinearFunction      LinearFunction::operator+(const LinearFunction& other) {
	LinearFunction      result(*this);
	for (unsigned int i = 0; i < 3; i++) {
		result.a[i] = a[i] + other.a[i];
	}
	return result;
}

LinearFunction&     LinearFunction::operator=(
	const LinearFunction& other) {
	for (int i = 0; i < 3; i++) {
		a[i] = other.a[i];
	}
	return *this;
}

/**
 * \brief read only access to coefficients
 */
double	LinearFunction::operator[](int i) const {
	if ((i < 0) || (i > 2)) {
		throw std::range_error("index out of range");
	}
	return a[i];
}

/**
 * \brief modifying access to coefficients
 */
double&	LinearFunction::operator[](int i) {
	if ((i < 0) || (i > 2)) {
		throw std::range_error("index out of range");
	}
	return a[i];
}

/**
 * \brief Compute the best possible coefficients from a data set
 */
void	LinearFunction::reduce(const std::vector<doublevaluepair>& values) {
	// build a linear system for coefficients a[3]
	int	m = values.size();
	int	n = 3;
	double	A[3 * m * n];
	double	b[m];
	int	line = 0;
	std::vector<doublevaluepair>::const_iterator	vp;
	for (vp = values.begin(); vp != values.end(); vp++, line++) {
		A[line + 0 * m] = vp->first.x() - center().x();
		A[line + 1 * m] = vp->first.y() - center().x();
		A[line + 2 * m] = 1;
		b[line] = vp->second;
	}

	// set up the lapack stuff
	char	trans = 'N';
	int	nrhs = 1;
	int	lda = m;
	int	ldb = m;
	int	lwork = -1;
	int	info = 0;

	// first find out how large we have to make the work area
	double	x;
	dgels_(&trans, &m, &n, &nrhs, A, &lda, b, &ldb, &x, &lwork, &info);
	if (info != 0) {
		std::string	msg = stringprintf("dgels cannot determine "
			"work area size: %d", info);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	lwork = x;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "work area size: %d", lwork);

	// now allocate memory and get the solution
	double	work[lwork];
	dgels_(&trans, &m, &n, &nrhs, A, &lda, b, &ldb, work, &lwork, &info);
	if (info != 0) {
		std::string	msg = stringprintf("dgels cannot solve "
			"equations: %d", info);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// copy the result vector
	for (int i = 0; i < 3; i++) {
		a[i] = b[i];
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Linear function found: %s",
		toString().c_str());
}

/**
 * \brief Create a linear function from a set of value pairs
 */
LinearFunction::LinearFunction(const ImagePoint& center, bool symmetric,
	const std::vector<LinearFunction::doublevaluepair>& values)
	: FunctionBase(center, symmetric) {
	a[0] = a[1] = a[2] = 0;
	reduce(values);
}

/**
 * \brief Text representation of a linear form
 */
std::string	LinearFunction::toString() const {
	return FunctionBase::toString()
		+ stringprintf("%f * dx + %f * dy + %f", a[0], a[1], a[2]);
}

//////////////////////////////////////////////////////////////////////
// QuadraticFunction implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief 
 */
QuadraticFunction::QuadraticFunction(const ImagePoint& center,
	bool symmetric) : LinearFunction(center, symmetric) {
	for (unsigned int i = 0; i < 3; i++) {
		q[i] = 0;
	}
}

QuadraticFunction::QuadraticFunction(const LinearFunction& lin)
	: LinearFunction(lin) {
	for (unsigned int i = 0; i < 3; i++) {
		q[i] = 0;
	}
}

double	QuadraticFunction::evaluate(const Point& point) const {
	double	value = LinearFunction::evaluate(point);
	if (gradient()) {
		double	deltax = point.x() - center().x();
		double	deltay = point.y() - center().y();
		value += q[0] * (sqr(deltax) + sqr(deltay));
		if (!symmetric()) {
			value += q[1] * deltax * deltay
				+ q[2] * (sqr(deltax) - sqr(deltay));
		}
	}
	return value;
}

double	QuadraticFunction::operator[](int i) const {
	if (i < 3) {
		return LinearFunction::operator[](i);
	}
	if (i > 5) {
		std::runtime_error("index out of range");
	}
	return q[i - 3];
}

double	QuadraticFunction::norm() const {
	double	s = sqr(LinearFunction::norm());
	for (int i = 0; i < 3; i++) {
		s += sqr(q[i]);
	}
	return sqrt(s);
}

void	QuadraticFunction::reduce(const std::vector<FunctionBase::doublevaluepair>& /* values */) {
	throw std::runtime_error("QuadraticFunction::reduce not implemented");
}

double&	QuadraticFunction::operator[](int i) {
	if (i < 3) {
		return LinearFunction::operator[](i);
	}
	if (i > 5) {
		std::runtime_error("index out of range");
	}
	return q[i - 3];
}

QuadraticFunction	QuadraticFunction::operator+(
	const QuadraticFunction& other) {
	QuadraticFunction	result(center(),
					symmetric() || other.symmetric());
	for (unsigned int i = 0; i < 6; i++) {
		result[i] += (*this)[i] + other[i];
	}
	return result;
}

QuadraticFunction	QuadraticFunction::operator+(
	const LinearFunction& other) {
	QuadraticFunction	result(center(),
					symmetric() || other.symmetric());
	for (unsigned int i = 0; i < 3; i++) {
		result[i] += (*this)[i] + other[i];
	}
	for (unsigned int i = 3; i < 5; i++) {
		result[i] += (*this)[i];
	}
	return result;
}

QuadraticFunction&	QuadraticFunction::operator=(
	const QuadraticFunction& other) {
	LinearFunction::operator=(other);
	for (unsigned int i = 0; i < 3; i++) {
		q[i] = other.q[i];
	}
	return *this;
}

QuadraticFunction&	QuadraticFunction::operator=(
	const LinearFunction& other) {
	LinearFunction::operator=(other);
	return *this;
}

std::string	QuadraticFunction::toString() const {
	return LinearFunction::toString()
		+ stringprintf("[%.6f, %.6f, %.6f]", q[0], q[1], q[2]);
} 

//////////////////////////////////////////////////////////////////////
// DegreeNFunctions
//////////////////////////////////////////////////////////////////////
/**
 * \brief 
 */
DegreeNFunction::DegreeNFunction(const DegreeNFunction& other)
	: QuadraticFunction(other), _n(other.n()) {
	m = new double[_n];
	for (int i = 0; i < _n; i++) {
		m[i] = other.m[i];
	}
}

DegreeNFunction::DegreeNFunction(const ImagePoint& center,
	bool symmetric, int n) : QuadraticFunction(center, symmetric), _n(n) {
	m = new double[_n];
	for (int i = 0; i < _n; i++) { m[i] = 0; }
}

DegreeNFunction::DegreeNFunction(const LinearFunction& lin, int n)
	: QuadraticFunction(lin), _n(n) {
	m = new double[_n];
	for (int i = 0; i < _n; i++) { m[i] = 0; }
}

DegreeNFunction::DegreeNFunction(const QuadraticFunction& q, int n)
	: QuadraticFunction(q), _n(n) {
	m = new double[_n];
	for (int i = 0; i < _n; i++) { m[i] = 0; }
}

DegreeNFunction::~DegreeNFunction() {
	delete m;
}

double	DegreeNFunction::evaluate(const Point& point) const {
	double	value = QuadraticFunction::evaluate(point);
	double	deltax = point.x() - center().x();
	double	deltay = point.y() - center().y();
	double	s = sqr(deltax) + sqr(deltay);
	double	p = s;
	for (int i = 0; i < _n; i++) {
		p *= s;
		value += m[i] * p;
	}
	return value;
}

double	DegreeNFunction::operator[](int i) const {
	if (i < 6) {
		return QuadraticFunction::operator[](i);
	}
	if (i > (5 + _n)) {
		return 0;
	}
	return m[i - 6];
}

double	DegreeNFunction::norm() const {
	double	s = sqr(QuadraticFunction::norm());
	for (int i = 0; i < _n; i++) {
		s += sqr(m[i]);
	}
	return sqrt(s);
}

void	DegreeNFunction::reduce(const std::vector<FunctionBase::doublevaluepair>& /* values */) {
	throw std::runtime_error("DegreeNFunction::reduce not implemented");
}

double&	DegreeNFunction::operator[](int i) {
	if (i < 6) {
		return QuadraticFunction::operator[](i);
	}
	if (i > (5 + _n)) {
		std::runtime_error("index out of range");
	}
	return m[i - 6];
}

DegreeNFunction	DegreeNFunction::operator+(
	const DegreeNFunction& other) {
	int	l = std::max(n(), other.n());
	DegreeNFunction	result(center(),
					symmetric() || other.symmetric(), l);
	for (int i = 0; i < 5 + l; i++) {
		result[i] += (*this)[i] + other[i];
	}
	return result;
}

DegreeNFunction	DegreeNFunction::operator+(
	const QuadraticFunction& other) {
	DegreeNFunction	result(center(),
					symmetric() || other.symmetric());
	for (unsigned int i = 0; i < 6; i++) {
		result[i] += (*this)[i] + other[i];
	}
	return result;
}

DegreeNFunction	DegreeNFunction::operator+(
	const LinearFunction& other) {
	DegreeNFunction	result(center(),
					symmetric() || other.symmetric());
	for (unsigned int i = 0; i < 3; i++) {
		result[i] += (*this)[i] + other[i];
	}
	for (unsigned int i = 3; i < 7; i++) {
		result[i] += (*this)[i];
	}
	return result;
}

DegreeNFunction&	DegreeNFunction::operator=(
	const DegreeNFunction& other) {
	QuadraticFunction::operator=(other);
	delete m;
	m = new double[other.n()];
	_n = other.n();
	for (int i = 0; i < _n; i++) {
		m[i] = other.m[i];
	}
	return *this;
}

DegreeNFunction&	DegreeNFunction::operator=(
	const LinearFunction& other) {
	LinearFunction::operator=(other);
	delete m;
	m = new double[1];
	_n = 1;
	for (unsigned int i = 3; i < 7; i++) {
		(*this)[i] = 0;
	}
	return *this;
}

std::string	DegreeNFunction::toString() const {
	std::ostringstream	out;
	out << QuadraticFunction::toString();
	out << "[";
	for (int i = 0; i < _n; i++) {
		if (i > 0) { out << ","; }
		out << stringprintf("%.6g", m[i]);
	}
	out << "]";
	return out.str();
} 


//////////////////////////////////////////////////////////////////////
// arithmetic operators for FunctionPtr
//////////////////////////////////////////////////////////////////////
FunctionPtr	operator+(const FunctionPtr& a, const FunctionPtr& b) {
	LinearFunction	*la = dynamic_cast<LinearFunction*>(&*a);
	LinearFunction	*lb = dynamic_cast<LinearFunction*>(&*b);
	QuadraticFunction	*qa = dynamic_cast<QuadraticFunction*>(&*a);
	QuadraticFunction	*qb = dynamic_cast<QuadraticFunction*>(&*b);
	DegreeNFunction	*da = dynamic_cast<DegreeNFunction*>(&*a);
	DegreeNFunction	*db = dynamic_cast<DegreeNFunction*>(&*b);
	if (da != NULL) {
		if (db != NULL) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "degree4 + degree4");
			return FunctionPtr(new DegreeNFunction(*da + *db));
		}
		if (qb != NULL) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "degree4 + quadratic");
			DegreeNFunction	d(*qb);
			return FunctionPtr(new DegreeNFunction(*da + d));
		}
		if (lb != NULL) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "degree4 + linear");
			DegreeNFunction	d(*lb);
			return FunctionPtr(new DegreeNFunction(*da + d));
		}
	}
	if (qa != NULL) {
		if (db != NULL) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "quadratic + degree4");
			DegreeNFunction	d(*qa);
			return FunctionPtr(new DegreeNFunction(d + *db));
		}
		if (qb != NULL) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "quadratic + quadratic");
			return FunctionPtr(new QuadraticFunction(*qa + *qb));
		}
		if (lb != NULL) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "quadratic + linear");
			QuadraticFunction	q(*lb);
			return FunctionPtr(new QuadraticFunction(*qa + q));
		}
	}
	if (la != NULL) {
		if (db != NULL) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "linear + degree4");
			DegreeNFunction	d(*la);
			return FunctionPtr(new DegreeNFunction(d + *db));
		}
		if (qb != NULL) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "linear + quadratic");
			QuadraticFunction	q(*la);
			return FunctionPtr(new QuadraticFunction(q + *qb));
		}
		if (lb != NULL) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "linear + linear");
			return FunctionPtr(new LinearFunction(*la + *lb));
		}
	}
	throw std::runtime_error("no matching combination for operator+");
}

} // namespace image
} // namespace astro
