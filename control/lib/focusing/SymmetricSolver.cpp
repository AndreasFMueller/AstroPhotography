/*
 * SymmetricSolver.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <stdexcept>
#include "SymmetricSolver.h"
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <cmath>
#include <iterator>
#include <algorithm>

namespace astro {
namespace focusing {

//////////////////////////////////////////////////////////////////////
// FunctionPoint implementation
//////////////////////////////////////////////////////////////////////

float	FunctionPoint::tolerance = 1e-7;


void	FunctionPoint::samex(const FunctionPoint& other) const {
	if (fabsf(x - other.x) > tolerance * (fabsf(x) + fabsf(other.x))) {
		std::string	msg = stringprintf("cannot operate at "
			"different x %f,%f", x, other.x);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
}

FunctionPoint	FunctionPoint::operator+(const FunctionPoint& other) const {
	samex(other);
	return FunctionPoint(x, y + other.y);
}

FunctionPoint	FunctionPoint::operator-(const FunctionPoint& other) const {
	samex(other);
	return FunctionPoint(x, y - other.y);
}

FunctionPoint	FunctionPoint::operator*(const FunctionPoint& other) const {
	samex(other);
	return FunctionPoint(x, y * other.y);
}

FunctionPoint	FunctionPoint::operator/(const FunctionPoint& other) const {
	samex(other);
	return FunctionPoint(x, y / other.y);
}

std::string	FunctionPoint::toString() const {
	return stringprintf("(%f,%f)", x, y);
}

//////////////////////////////////////////////////////////////////////
// FunctionPointPair implementation
//////////////////////////////////////////////////////////////////////

FunctionPointPair::FunctionPointPair(const FunctionPoint& p1,
	const FunctionPoint& p2)
	: std::pair<FunctionPoint, FunctionPoint>(p1, p2) {
#if 0
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new function point pair %s, %s",
		p1.toString().c_str(), p2.toString().c_str());
#endif
}

void	FunctionPointPair::contains(float x) const {
	if ((first.x <= x) && (x <= second.x)) {
		return;
	}
	std::string	msg = stringprintf("%f not contained in [%f,%f]",
		x, first.x, second.x);
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

float	FunctionPointPair::mx() const {
	return (second.x + first.x) / 2.;
}

float	FunctionPointPair::mf() const {
	return (second.y + first.y) / 2.;
}

float	FunctionPointPair::deltax() const {
	return (second.x - first.x) / 2.;
}

float	FunctionPointPair::deltaf() const {
	return (second.y - first.y) / 2.;
}

float	FunctionPointPair::t(float x) const {
	contains(x);
	return (x - mx()) / deltax();
}

float	FunctionPointPair::x(float t) const {
	return mx() + t * deltax();
}

float	FunctionPointPair::f(float t) const {
	return mf() + t * deltaf();
}

float	FunctionPointPair::interpolate(float x) const {
	return f(t(x));
}

std::string	FunctionPointPair::toString() const {
	return stringprintf("[%s, %s]",
		first.toString().c_str(), second.toString().c_str());
}

float	FunctionPointPair::integrate() const {
	return deltax() * mf();
}

static inline float	sqr(float x) {
	return x * x;
}

float	FunctionPointPair::integrate2() const {
	return deltax() * (sqr(mf()) + sqr(deltaf()) / 3);
}

//////////////////////////////////////////////////////////////////////
// Function implementation
//////////////////////////////////////////////////////////////////////

FunctionPointPair	Function::paircontaining(float x) const {
	Function::const_iterator	i = begin();
	FunctionPoint	previous = *i;
	while (i != end()) {
		i++;
		if ((previous.x <= x) && (x <= i->x)) {
			return FunctionPointPair(previous, *i);
		}
		previous = *i;
	}
	std::string	msg = stringprintf("no interval containing %f", x);
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

float	Function::maxx() const {
	return rbegin()->x;
}

float 	Function::minx() const {
	return begin()->x;
}

bool	Function::contains(float x) const {
	return (begin()->x <= x) && (x <= rbegin()->x);
}

float	Function::operator()(const float x) const {
	return paircontaining(x).interpolate(x);
}

float	Function::operator[](const size_t index) const {
	if (index >= size()) {
		std::string	msg = stringprintf("index %d out of range [0,%d]", index, size() - 1);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::range_error(msg);
	}
	Function::const_iterator	i = begin();
	std::advance(i, index);
	return i->y;
}

Function	Function::mirror(float x0) const {
	Function	result;
	std::for_each(begin(), end(), 
		[&result, x0](const FunctionPoint& p) {
			result.insert(FunctionPoint(x0 - p.x, p.y));
		}
	);
	return result;
}

void	Function::add(float x) {
	FunctionPointPair	p = paircontaining(x);
	if ((p.first.x == x) || (p.second.x == x)) {
		return;
	}
	insert(FunctionPoint(x, this->operator()(x)));
}

void	Function::add(const Function& other) {
	std::for_each(other.begin(), other.end(),
		[this](const FunctionPoint& p) {
			if (this->contains(p.x)) {
				this->add(p.x);
			}
		}
	);
}

std::string	Function::toString() const {
	std::string	result;
	std::for_each(begin(), end(),
		[&result](const FunctionPoint& p) {
			result.append(" ");
			result.append(p.toString());
		}
	);
	return result;
}

Function	Function::restrict(const Function& other) const {
	Function	result;
	float	min = std::max(minx(), other.minx());
	float	max = std::min(maxx(), other.maxx());
	if (min > max) {
		// XX better error message
		throw std::runtime_error("no intersection");
	}
	result.insert(FunctionPoint(min, (*this)(min)));
	result.insert(FunctionPoint(max, (*this)(max)));
	std::for_each(begin(), end(),
		[&result](const FunctionPoint& p) {
			if (result.contains(p.x)) {
				result.insert(p);
			}
		}
	);
	std::for_each(other.begin(), other.end(),
		[&result](const FunctionPoint& p) {
			if (result.contains(p.x)) {
				result.add(p.x);
			}
		}
	);
	return result;
}

Function	Function::operator+(const Function& other) const {
	Function	a = restrict(other);
	Function	b = other.restrict(*this);
	Function	result;
	std::for_each(a.begin(), a.end(),
		[&result,&b](const FunctionPoint& p) {
			result.insert(FunctionPoint(p.x, p.y + b(p.x)));
		}
	);
	return result;
}

Function	Function::operator-(const Function& other) const {
	Function	a = restrict(other);
	Function	b = other.restrict(*this);
	Function	result;
	std::for_each(a.begin(), a.end(),
		[&result,&b](const FunctionPoint& p) {
			result.insert(FunctionPoint(p.x, p.y - b(p.x)));
		}
	);
	return result;
}

Function	Function::operator*(const Function& other) const {
	Function	a = restrict(other);
	Function	b = other.restrict(*this);
	Function	result;
	std::for_each(a.begin(), a.end(),
		[&result,&b](const FunctionPoint& p) {
			result.insert(FunctionPoint(p.x, p.y * b(p.x)));
		}
	);
	return result;
}

Function	Function::operator/(const Function& other) const {
	Function	a = restrict(other);
	Function	b = other.restrict(*this);
	Function	result;
	std::for_each(a.begin(), a.end(),
		[&result,&b](const FunctionPoint& p) {
			result.insert(FunctionPoint(p.x, p.y / b(p.x)));
		}
	);
	return result;
}

float	Function::integrate() const {
	float	result = 0;
	FunctionPoint	previous = *begin();
	std::for_each(begin(), end(),
		[&result, &previous](const FunctionPoint p) {
			FunctionPointPair	fpp(previous, p);
			result += fpp.integrate();
			previous = p;
		}
	);
	return result;
}

float	Function::integrate2() const {
	float	result = 0;
	FunctionPoint	previous = *begin();
	std::for_each(begin(), end(),
		[&result, &previous](const FunctionPoint p) {
			FunctionPointPair	fpp(previous, p);
			result += fpp.integrate2();
			previous = p;
		}
	);
	return result;
}

} // namespace focusing
} // namespace astro
