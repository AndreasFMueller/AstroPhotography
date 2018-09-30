/*
 * AstroCoordinates.h -- Classes for compuations in various coordinate systems
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroCoordinates_h
#define _AstroCoordinates_h

#include <AstroTypes.h>

#include <math.h>
#include <string>
#include <time.h>

namespace astro {

/**
 * \brief Angle abstraction
 *
 * Angles are measured in different units of measure, and this class takes
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
	Angle	reduced(const double base = 0) const;
static double	hours_to_radians(const double h);
static double	degrees_to_radians(const double d);
static double	radians_to_hours(const double r);
static double	radians_to_degrees(const double r);
static Angle	hms_to_angle(const std::string& hms);
static Angle	dms_to_angle(const std::string& dms);
	bool	operator<(const Angle& other) const;
	bool	operator<=(const Angle& other) const;
	bool	operator>(const Angle& other) const;
	bool	operator>=(const Angle& other) const;
	bool	operator==(const Angle& other) const;
	bool	operator!=(const Angle& other) const;
static const Angle right_angle;
};

Angle	operator*(double l, const Angle& a);

double	cos(const Angle& a);
double	sin(const Angle& a);
double	tan(const Angle& a);
double	cot(const Angle& a);
double	sec(const Angle& a);
double	csc(const Angle& a);

Angle	arccos(double x);
Angle	arcsin(double x);
Angle	arctan(double x);
Angle	arctan2(double y, double x);

/**
 * \brief Class combining two angles
 *
 * This class is used as the base class for spherical coordinates on
 * earth and on the celestial sphere
 */
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
	TwoAngles	operator+(const TwoAngles& other) const;
	TwoAngles	operator-(const TwoAngles& other) const;
	bool	operator==(const TwoAngles& other) const;
	bool	operator!=(const TwoAngles& other) const;
};

class LongLat;
class RaDec;

/**
 * \brief Spherical coordinates class
 */
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

Angle	operator-(const SphericalCoordinates& s1, const SphericalCoordinates& s2);

class Vector;
/**
 * \brief Class for right ascension and declination
 *
 * This class is used to describe points on the celestial sphere
 */
class RaDec : public TwoAngles {
public:
	RaDec() { }
	RaDec(const TwoAngles& ta) : TwoAngles(ta) { }
	RaDec(const Angle& ra, const Angle& dec) : TwoAngles(ra, dec) { }
	RaDec(const SphericalCoordinates& spherical)
		: TwoAngles(spherical.phi(), Angle(M_PI / 2) - spherical.theta()) { }
	RaDec(const Vector& vector);
	const Angle&	ra() const { return a1(); }
	Angle&	ra() { return a1(); }
	const Angle&	dec() const { return a2(); }
	Angle&	dec() { return a2(); }
	bool	operator<(const RaDec& other) const;
	bool	operator>(const RaDec& other) const;
	bool	operator<=(const RaDec& other) const;
	bool	operator>=(const RaDec& other) const;
	RaDec	operator+(const RaDec& other) const;
	RaDec	operator-(const RaDec& other) const;
	virtual std::string	toString() const;
static const RaDec	north_pole;
static const RaDec	south_pole;
};

//class	UnitVector;
/**
 * \brief vector pointing from the center of the sphere to a point
 */
class	Vector {
protected:
	double	_x[3];
public:
	Vector();
	Vector(const double x[3]);
	Vector	cross(const Vector& other) const;
	double	x() const { return _x[0]; }
	double	y() const { return _x[1]; }
	double	z() const { return _x[2]; }
	double	abs() const;
	double	operator*(const Vector& other) const;
	Vector	operator+(const Vector& other) const;
	Vector	operator-(const Vector& other) const;
	Vector	operator-() const;
	Vector	operator*(double l) const;
	std::string	toString() const;
	Vector	normalized() const;
};

/**
 * \brief Class describing a unit vector
 */
class	UnitVector : public Vector {
public:
	UnitVector();
	UnitVector(const SphericalCoordinates& spherical);
	UnitVector(const RaDec& radec);
	UnitVector(const double x[3]);
	UnitVector(const Vector& other);
	UnitVector&	operator=(const Vector& other);
	Angle	angle(const UnitVector& other) const;
	Vector	operator()(const Vector& other) const;
};

/**
 * \brief Altazimut class based on two angles
 */
class	AzmAlt : public TwoAngles {
public:
	AzmAlt() { }
	AzmAlt(const Angle& azm, const Angle& alt) : TwoAngles(azm, alt) { }
	const Angle&	azm() const { return a1(); }
	Angle&	azm() { return a1(); }
	const Angle&	alt() const { return a2(); }
	Angle&	alt() { return a2(); }
};

/**
 * \brief Longitude/Latitude class for points on earth
 */
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

/**
 * \brief Baryzentric coordinate points
 */
class BarycentricPoint : public Point {
public:
	BarycentricPoint(double _w1, double _w2, double _w3);
	double	w1() const { return x(); }
	double	w2() const { return y(); }
	double	w3() const { return 1 - w1() - w2(); }
	std::string	toString() const;
	bool	inside() const;
};

/**
 * \brief Barycentric coordinates
 */
class BarycentricCoordinates {
	Point	_p1, _p2, _p3;
	double	b[9];
public:
	const Point&	p1() const { return _p1; }
	const Point&	p2() const { return _p2; }
	const Point&	p3() const { return _p3; }
	BarycentricCoordinates(const Point& p1, const Point& p2,
		const Point& p3);
	BarycentricPoint	operator()(const Point& point) const;
	Point	operator()(const BarycentricPoint& barycentricpoint) const;
	std::string	toString() const;
	bool	inside(const Point& point) const;
};

/**
 * \brief Coordinate Conversion for 
 */
class ImageCoordinates {
	RaDec	_center;
	Angle	_angular_resolution;
	Angle	_azimut;
	bool	_mirror;
public:
	ImageCoordinates(const RaDec& center, const Angle& angular_resolution,
		const Angle& azimut, bool mirror = false);
	ImageCoordinates(const RaDec& center, const Angle& angular_resolution,
		bool mirror = false);
	RaDec	offset(const Point& offset) const;
	RaDec	operator()(const Point& offset) const;
};

/**
 * \brief Julian Date
 */
class JulianDate {
	double	_H;
	double	_T;
public:
	virtual void	update(time_t when);
	void	update();
	JulianDate();
	JulianDate(time_t when);
	virtual ~JulianDate() { }
	double	T() const { return _T; }
	Angle	GMST() const;
};

/**
 * \brief Converter class from RA/DEC to azimutal coordinates
 */
class AzmAltConverter : public JulianDate {
	LongLat	_longlat;
	Angle	_lmst;
public:
	virtual void	update(time_t when);
	virtual void	update();
	AzmAltConverter(time_t when, const LongLat& longlat);
	AzmAltConverter(const LongLat& longlat);
	virtual ~AzmAltConverter() { }
	AzmAlt	operator()(const RaDec& radec);
	Angle	LMST() const { return _lmst; }
	RaDec	inverse(const AzmAlt& azmalt);
};

} // namespace astro

#endif /* _AstroCoordinates_h */
