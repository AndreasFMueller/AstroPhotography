/*
 * AstroCoordinates.h -- Classes for compuations in various coordinate systems
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroCoordinates_h
#define _AstroCoordinates_h

#include <math.h>
#include <string>

namespace astro {

/**
 * \brief Angle abstraction
 *
 * ANgles are measured in different units of measure, and this class takes
 * care of these units by providing accessor functions that perform the
 * conversions. Internally, the angle is always kept in radians.
 */
class Angle {
	double	_angle; // in radians
protected:
	virtual void	reduce(double base = 0);
public:
	Angle(double angle = 0);
	double	degrees() const;
	void	degrees(double degrees);
	std::string	dms(const char separator = ':') const;
	double	hours() const;
	void	hours(double hours);
	std::string	hms(const char separator = ':') const;
	double	radians() const { return _angle; }
	void	radians(double radians) { _angle = radians; }
	Angle	operator+(const Angle& other) const;
	Angle	operator-(const Angle& other) const;
	Angle	operator*(const double& other) const;
	Angle	reduced(const double base = 0);
static double	hours_to_radians(const double h);
static double	degrees_to_radians(const double d);
static double	radians_to_hours(const double r);
static double	radians_to_degrees(const double r);
	bool	operator<(const Angle& other) const;
	bool	operator<=(const Angle& other) const;
	bool	operator>(const Angle& other) const;
	bool	operator>=(const Angle& other) const;
};

double	cos(const Angle& a);
double	sin(const Angle& a);
double	tan(const Angle& a);
double	cot(const Angle& a);
double	sec(const Angle& a);
double	csc(const Angle& a);

class TwoAngles {
	Angle	_a1;
	Angle	_a2;
public:
	TwoAngles() { }
	TwoAngles(const Angle& a1, const Angle& a2) : _a1(a1), _a2(a2) { }
	const Angle&	a1() const { return _a1; }
	Angle&	a1() { return _a1; }
	const Angle&	a2() const { return _a2; }
	Angle&	a2() { return _a2; }
	const Angle&	operator[](int i) const;
	Angle&	operator[](int i);
};

class LongLat;
class RaDec;

class SphericalCoordinates : public TwoAngles {
public:
	SphericalCoordinates() { }
	SphericalCoordinates(const Angle& phi, const Angle& theta)
		: TwoAngles(phi, theta) { }
	SphericalCoordinates(const LongLat& longlat);
	SphericalCoordinates(const RaDec& radec);
	const Angle&	phi() const { return a1(); }
	Angle&	phi() { return a1(); }
	const Angle&	theta() const { return a2(); }
	Angle&	theta() { return a2(); }
};

class	UnitVector {
	double	_x[3];
public:
	UnitVector(const SphericalCoordinates& spherical);
	double	operator*(const UnitVector& other) const;
	double	x() const { return _x[0]; }
	double	y() const { return _x[1]; }
	double	z() const { return _x[2]; }
	Angle	operator-(const UnitVector& other) const;
};

Angle	operator-(const SphericalCoordinates& s1, const SphericalCoordinates& s2);

class RaDec : public TwoAngles {
public:
	RaDec() { }
	RaDec(const Angle& ra, const Angle& dec) : TwoAngles(ra, dec) { }
	RaDec(const SphericalCoordinates& spherical)
		: TwoAngles(spherical.phi(), Angle(M_PI / 2) - spherical.theta()) { }
	const Angle&	ra() const { return a1(); }
	Angle&	ra() { return a1(); }
	const Angle&	dec() const { return a2(); }
	Angle&	dec() { return a2(); }
	bool	operator<(const RaDec& other) const;
	bool	operator>(const RaDec& other) const;
	bool	operator<=(const RaDec& other) const;
	bool	operator>=(const RaDec& other) const;
};

class	AzmAlt : public TwoAngles {
public:
	AzmAlt() { }
	AzmAlt(const Angle& azm, const Angle& alt) : TwoAngles(azm, alt) { }
	const Angle&	azm() const { return a1(); }
	Angle&	azm() { return a1(); }
	const Angle&	alt() const { return a2(); }
	Angle&	alt() { return a2(); }
};

class	LongLat : public TwoAngles {
public:
	LongLat() { }
	LongLat(const Angle& longitude, const Angle& latitude)
		: TwoAngles(longitude, latitude) { }
	LongLat(const SphericalCoordinates& spherical)
		: TwoAngles(spherical.phi(), Angle(M_PI / 2) - spherical.theta()) { }
	const Angle&	longitude() const { return a1(); }
	Angle&	longitude() { return a1(); }
	const Angle&	latitude() const { return a2(); }
	Angle&	latitude() { return a2(); }
};

} // namespace astro

#endif /* _AstroCoordinates_h */
