/*
 * BrennerSolver.cpp
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFocus.h>
#include "FocusSolvers.h"
#include <lapack.h>

namespace astro {
namespace focusing {

#define	N	4

class BrennerFunctionBase {
protected:
	FocusItems	_focusitems;
public:
	const FocusItems&	focusitems() const { return _focusitems; }
protected:
	double	minimum;
	double	maximum;
	int	maxposition;
public:
	BrennerFunctionBase(const FocusItems& focusitems);

	virtual double	operator()(double *a, double x) const = 0;
	virtual void	initial(double *a) const = 0;
	virtual void	correct(double *a, double *v) const = 0;

	double	operator()(double *a) const;
	double	firstderivative(double *a, int i) const;
	double	secondderivative(double *a, int i, int j) const;
	void	derivative(double *a, double *d) const;
	void	hessian(double *a, double *h) const;
	std::string	info(const std::string& name, double *a) const;

	void	print(double *a, std::ostream& out) const;
};

BrennerFunctionBase::BrennerFunctionBase(const FocusItems& focusitems) {
	// find the position value for the maximum, the maximum and the
	// minimum;
	minimum = std::numeric_limits<double>::max();
	maximum = 0;
	maxposition = -1;
	FocusItems::const_iterator	i;
	for (i = focusitems.begin(); i != focusitems.end(); i++) {
		double	value = i->value();
		if (value > maximum) {
			maximum = value;
			maxposition = i->position();
		}
		if (value < minimum) {
			minimum = value;
		}
	}
	// now normalize the function
	for (i = focusitems.begin(); i != focusitems.end(); i++) {
		FocusItem	focusitem(i->position(), i->value() / maximum);
		_focusitems.insert(focusitem);
	}
	minimum /= maximum;
	maximum = 1;
}

std::string	BrennerFunctionBase::info(const std::string& name, double *a) const {
	std::string	as;
	for (int i = 0; i < N; i++) {
		as += stringprintf("%s[%d]=%g ", name.c_str(), i, a[i]);
	}
	return as;
}

double	BrennerFunctionBase::firstderivative(double *a, int i) const {
	double	a2[N];
	double	a1[N];
	for (int i = 0; i < N; i++) {
		a2[i] = a[i];
		a1[i] = a[i];
	}
	double	h = 0.0001 * a[i];
	a2[i] += h;
	a1[i] -= h;
	double	delta = (*this)(a2) - (*this)(a1);
	return delta / (2 * h);
}

void	BrennerFunctionBase::derivative(double *a, double *d) const {
	for (int i = 0; i < N; i++) {
		d[i] = firstderivative(a, i);
	}
}

double	BrennerFunctionBase::secondderivative(double *a, int i, int j) const {
	double	h = 0.0001 * a[j];
	double	a2[N], a1[N];
	for (int i = 0; i < N; i++) {
		a2[i] = a[i];
		a1[i] = a[i];
	}
	a2[j] += h;
	a1[j] -= h;
	double	delta = firstderivative(a2, i) - firstderivative(a1, i);
	return delta / (2 * h);
}

void	BrennerFunctionBase::hessian(double *a, double *h) const {
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			h[i + N * j] = secondderivative(a, i, j);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "D2(%d,%d) = %f",
				i, j, h[i + N * j]);
		}
	}
}

void	BrennerFunctionBase::print(double *a, std::ostream& out) const {
	out << "position,value,fitted" << std::endl;
	FocusItems::const_iterator	i;
	for (i = _focusitems.begin(); i != _focusitems.end(); i++) {
		out << i->position() << ",";
		out << i->value() << ",";
		out << this->operator()(a, i->position()) << std::endl;
	}
}

double	BrennerFunctionBase::operator()(double *a) const {
	double	sum = 0;
	FocusItems::const_iterator	i;
	for (i = _focusitems.begin(); i != _focusitems.end(); i++) {
		double	x = i->position();
		double	y = this->operator()(a, x) - i->value();
		sum += y * y;
	}
	return sum;
}

/**
 * \brief BrennerFunction class with quadratic solution
 */
class BrennerFunction : public BrennerFunctionBase {
public:
	BrennerFunction(const FocusItems& focusitems)
		: BrennerFunctionBase(focusitems) { }
	virtual double	operator()(double *a, double x) const;
	virtual void	initial(double *a) const;
	virtual void	correct(double *a, double *v) const;
};

double	BrennerFunction::operator()(double *a, double x) const {
	return a[2] / (1 + a[3] * pow(fabs(x - a[0]), 2)) + a[1];
}

void	BrennerFunction::initial(double *a) const {
	a[0] = maxposition;
	a[1] = minimum;
	a[2] = maximum - minimum;
	a[3] = 0.00000025;
}

void	BrennerFunction::correct(double *a, double *v) const {
	for (int i = 0; i < N; i++) {
		a[i] -= v[i];
	}
	if (a[0] < 0) {
		a[0] = 0;
	}
	if (a[0] > 65537) {
		a[0] = 65537;
	}
	if (a[1] < 0) {
		a[1] = 0;
	}
	if ((a[2] < 0) && ((a[2] + a[1]) > 1.2 * maximum)) {
		a[2] = maximum - a[1];
	}
}

int	BrennerSolver::position(const FocusItems& focusitems) {
	BrennerFunction	b(focusitems);

	// find initial values for Brenner function coefficients
	double	a[N];
	b.initial(a);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "initial values: %s",
		b.info("a", a).c_str());

	// now compute the values of the deriviative
	int	count = 0;
	while (count < 20) {
		double	v[N];
		b.derivative(a, v);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", b.info("v", v).c_str());

		// compute the hessian
		double	h[N * N];
		b.hessian(a, h);

		// solve the equation
		int	n = N, nrhs = 1, lda = N, ldb = N, info;
		int	ipiv[N];
		dgesv_(&n, &nrhs, h, &lda, ipiv, v, &ldb, &info);

		if (0 != info) {
			throw std::runtime_error("cannot solve newton");
		}
		b.correct(a, v);

		debug(LOG_DEBUG, DEBUG_LOG, 0, "a-values: %s",
			b.info("a", a).c_str());
		count++;
	}

	b.print(a, std::cout);
	
	return a[0];
}

BrennerSolver::BrennerSolver() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create a new BrennerSolver");
}

} // namespace focusing
} // namespace astro
