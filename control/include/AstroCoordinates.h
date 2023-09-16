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
public:
	virtual void	reduce(double base = 0);
	typedef enum { Radians, Degrees, Hours, Revolutions, ArcMinutes, ArcSeconds, Minutes, Seconds } unit;
	Angle(double angle = 0, unit u = Radians);
	Angle(double x, double y);
	Angle(const std::string& a, unit u = Degrees);
	virtual ~Angle() { }
	double	degrees() const;
	double	arcminutes() const;
	double	arcseconds() const;
	void	degrees(double degrees);
	void	arcminutes(double arcminutes);
	void	arcseconds(double arcseconds);
	std::string	dms(const char separator = ':', int precision = 3) const;
	double	hours() const;
	double	minutes() const;
	double	seconds() const;
	void	hours(double hours);
	void	minutes(double minutes);
	void	seconds(double seconds);
	std::string	hms(const char separator = ':', int precision = 3) const;
	double	radians() const { return _angle; }
	void	radians(double radians) { _angle = radians; }
	double	revolutions() const;
	void	revolutions(double revolutions);
	double	value(unit u) const;
	double	sin() const;
	double	cos() const;
	double	tan() const;
	Angle	operator+(const Angle& other) const;
	Angle	operator-(const Angle& other) const;
	Angle	operator*(const double& other) const;
	double	operator/(const Angle& other) const;
	Angle	operator-() const;
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
static const Angle ecliptic_angle;
static Angle	ecliptic(double T);
};

Angle	operator*(double l, const Angle& a);

std::ostream&	operator<<(std::ostream& out, const Angle& angle);

double	cos(const Angle& a);
double	sin(const Angle& a);
double	tan(const Angle& a);
double	cot(const Angle& a);
double	sec(const Angle& a);
double	csc(const Angle& a);
Angle	abs(const Angle& a);

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
	TwoAngles	operator*(double t) const;
	bool	operator==(const TwoAngles& other) const;
	bool	operator!=(const TwoAngles& other) const;
	bool	operator<(const TwoAngles& other) const;
	std::string	toString(Angle::unit unit = Angle::Degrees) const;
};

std::ostream&	operator<<(std::ostream& out, const TwoAngles& angles);

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
class Ecliptic;
class Precession;
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
		: TwoAngles(spherical.phi(),
			Angle(M_PI / 2) - spherical.theta()) { }
	RaDec(const Vector& vector);
	RaDec(const Ecliptic& ecliptic);
	RaDec(const std::string& radecstring);
	virtual ~RaDec() { }
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
	RaDec	operator*(double t) const;
	virtual std::string	toString() const;
static const RaDec	north_pole;
static const RaDec	south_pole;
	Ecliptic	ecliptic() const;
	void	precess(const Precession& precession);
	RaDec	exp(const Angle& position_angle, const Angle& radius) const;
	Angle	distance(const RaDec& other) const;
	double	scalarproduct(const RaDec& other) const;
};

std::ostream&	operator<<(std::ostream& out, const RaDec& radec);

/**
 * \brief Class to parametrize a great circle
 */
class GreatCircle {
	RaDec	_A;
	RaDec	_B;
	Angle	_a;
	Angle	_b;
	Angle	_c;
	Angle	_alpha;
	Angle	_beta;
	Angle	_gamma;
	int	_sign;
public:
	GreatCircle() { }
	GreatCircle(const RaDec& A, const RaDec& B);
	Angle	c(double t) const;
	Angle	a(double t) const;
	Angle	gamma(double t) const;
	RaDec	operator()(double t) const;
};

/**
 * \brief Ecliptic coordinates
 */
class Ecliptic : public TwoAngles {
public:
	Ecliptic() { }
	Ecliptic(const TwoAngles& ta) : TwoAngles(ta) { }
	Ecliptic(const Angle& lambda, const Angle& beta)
		: TwoAngles(lambda, beta) { }
	Ecliptic(const RaDec& radec);
	const Angle&	lambda() const { return a1(); }
	Angle&	lambda() { return a1(); }
	const Angle&	beta() const { return a2(); }
	Angle&	beta() { return a2(); }
	RaDec	radec() const;
	void	precess(const Precession& precession);
	virtual std::string	toString() const;
};

/**
 * \brief Precession operator
 */
class Precession {
	Angle	precessionangle;
	void	setup(time_t when);
public:
	Precession();
	Precession(time_t when);
	Precession(double years);
	Ecliptic	operator()(const Ecliptic& ecliptic) const;
	RaDec	operator()(const RaDec& radec) const;
};

class Rotation3D;

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
	Vector(double X, double Y, double Z);
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
friend class Rotation3D;
	static Vector Ex();
	static Vector Ey();
	static Vector Ez();
};

std::ostream&	operator<<(std::ostream& out, const Vector& vector);

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
	static UnitVector Ex();
	static UnitVector Ey();
	static UnitVector Ez();
};

/**
 * \brief Class implementation rotation around an axis
 */
class	Rotation3D {
	double	m[3][3];
	void	setup(const UnitVector& u, const Angle& a);
public:
	Rotation3D(const Vector& a);
	Rotation3D(const UnitVector& u, const Angle& a);
	Rotation3D(char axis, const Angle& a);
	Vector	operator()(const Vector& v) const;
	UnitVector	operator()(const UnitVector& v) const;
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

	std::string	toString() const;
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
	Point	offset(const RaDec& direction) const;
	Point	operator()(const RaDec& direction) const;
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
	double	H() const { return _H; }
	double	years() const;
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
	AzmAlt	operator()(const RaDec& radec) const;
	Angle	LMST() const { return _lmst; }
	RaDec	inverse(const AzmAlt& azmalt) const;
	Angle	hourangle(const RaDec& radec) const;
};

/**
 * \brief Converter for pixelsize/focallength into arcsec per pixel
 */
class AngularSize : public Angle {
public:
	AngularSize(double pixelsize, double focallength);
	AngularSize(const Angle& angle);
	double	scaledPixel(double distance) const;
};

double	operator/(double r, const AngularSize& s);
double	operator/(const Angle& angle, const AngularSize& s);

namespace utils {

/**
 * \brief Utility class to compute coordinate grids on the celestial sphere
 *
 * This is used by the StarChartWidget class in the gui to do the computation
 * for grid displays.
 */
class GridCalculator {
	RaDec	_center;
	Size	_frame;
	double	_pixels_per_degree;
public:
	GridCalculator(const RaDec& center, const Size& frame,
		double pixels_per_degree);
	const RaDec&	center() const { return _center; }
	const Size&	frame() const { return _frame; }
	double	pixels_per_degree() const { return _pixels_per_degree; }
private:
	RaDec	_gridzero;
	RaDec	_stepsizes;
	int	_minra;
	int	_maxra;
	int	_mindec;
	int	_maxdec;
	bool	_pole_in_frame;
public:
	const RaDec&	gridzero() const { return _gridzero; }
	const RaDec&	stepsizes() const { return _stepsizes; }
	int	maxra() const { return _maxra; }
	int	minra() const { return _minra; }
	int	maxdec() const { return _maxdec; }
	int	mindec() const { return _mindec; }
	void	gridsetup(double pixelstep);
	// compute grid points
	Angle	ra(int ra) const;
	Angle	dec(int dec) const;
	RaDec	gridpoint(int ra, int dec) const;
	TwoAngles	angleRangeRA(int dec) const;
	TwoAngles	angleRangeDEC(int ra) const;
};

} // namespace utils
} // namespace astro

#endif /* _AstroCoordinates_h */
