/*
 * SymmetricSolver.h -- definitions for the symmetry based solver
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SymmetricSolver_h
#define _SymmetricSolver_h

#include <set>
#include <string>

namespace astro {
namespace focusing {

class FunctionPoint {
	static float	tolerance;
	void	samex(const FunctionPoint& other) const;
public:
	float	x;
	float	y;
	FunctionPoint(float _x, float _y) : x(_x), y(_y) { }
	FunctionPoint(const FunctionPoint& other) { x = other.x; y = other.y; }
	bool	operator<(const FunctionPoint& other) const {
		return x < other.x;
	}
	bool	operator==(const FunctionPoint& other) const {
		return x == other.x;
	}
	FunctionPoint	operator+(const FunctionPoint& other) const;
	FunctionPoint	operator-(const FunctionPoint& other) const;
	FunctionPoint	operator*(const FunctionPoint& other) const;
	FunctionPoint	operator/(const FunctionPoint& other) const;
	std::string	toString() const;
};

class FunctionPointPair : public std::pair<FunctionPoint, FunctionPoint> {
	void	contains(float x) const;
public:
	FunctionPointPair(const FunctionPoint& p1, const FunctionPoint& p2);
	float	mx() const;
	float	mf() const;
	float	deltax() const;
	float	deltaf() const;
	float	t(float x) const;
	float	x(float t) const;
	float	f(float t) const;
	float	interpolate(float x) const;
	std::string	toString() const;
	float	integrate() const;
	float	integrate2() const;
};

class Function : public std::set<FunctionPoint> {
	FunctionPointPair	paircontaining(float x) const;
public:
	float	operator()(const float x) const;
	float	operator[](const size_t index) const;
	float	maxx() const;
	float	minx() const;
	bool	contains(float x) const;
	Function	mirror(float x0) const;
	void	add(float x);
	void	add(const Function& other);
	std::string	toString() const;
	Function	restrict(const Function& other) const;
	Function	operator+(const Function& other) const;
	Function	operator-(const Function& other) const;
	Function	operator*(const Function& other) const;
	Function	operator/(const Function& other) const;
	float	integrate() const;
	float	integrate2() const;
};

} // namespace focusing
} // namespace astro

#endif /* _SymmetricSolver_h */


