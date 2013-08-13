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
	void	setDegrees(double degrees);
	double	hours() const;
	void	setHours(double hours);
	double	radians() const;
	void	setRadians(double radians);
	Angle	operator+(const Angle& other) const;
	Angle	operator-(const Angle& other) const;
	Angle	operator*(const double& other) const;
};

class RaDec {
	Angle	_ra;
	Angle	_dec;
public:
	Angle	ra() const;
	Angle	dec() const;
}




} // namespace astro

#endif /* _AstroCoordinates_h */
