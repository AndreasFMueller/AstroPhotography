/*
 * AstroCoordinates.h -- Classes for compuations in various coordinate systems
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroCoordinates_h
#define _AstroCoordinates_h

namespace astro {

class Angle {
	double	_angle; // in radians
protected:
	virtual void	reduce();
public:
	Angle(double angle = 0);
	double	degrees() const;
	void	degrees(double degrees);
	double	hours() const;
	void	hours(double hours);
	double	radians() const { return _angle; }
	void	radians(double radians) { _angle = radians; }
	Angle	operator+(const Angle& other) const;
	Angle	operator-(const Angle& other) const;
	Angle	operator*(const double& other) const;
};

class RaDec {
	Angle	_ra;
	Angle	_dec;
public:
	RaDec(const Angle& ra, const Angle& dec) : _ra(ra), _dec(dec) { }
	const Angle&	ra() const { return _ra; }
	const Angle&	dec() const { return _dec; }
};

class	AzmAlt {
	Angle	_azm;
	Angle	_alt;
public:
	AzmAlt(const Angle& azm, const Angle& alt) : _azm(azm), _alt(alt) { }
	const Angle&	azm() const { return _azm; }
	const Angle&	alt() const { return _alt; }
};

} // namespace astro

#endif /* _AstroCoordinates_h */
