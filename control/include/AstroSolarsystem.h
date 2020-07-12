/*
 * AstroSolarsystem.h
 *
 * Some functions and classes that can be used to find the position
 * the sun, the moon and the planets. Currently, these functions are
 * very rudimentary, a more usefull version will include a reimplementation
 * of the code from the book of Pfleger and Montenbruck
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _AstroSolarsystem_h
#define _AstroSolarsystem_h

#include <AstroCoordinates.h>
#include <list>
#include <memory>

namespace astro {
namespace solarsystem {

/**
 * \brief Base class for solar system bodies
 */
class SolarsystemBody {
	std::string	_name;
	virtual RaDec	ephemerisT(double T0) = 0;
public:
	const std::string&	name() const { return _name; }
	SolarsystemBody(const std::string& name);
	virtual	~SolarsystemBody();
	RaDec	ephemeris(time_t when);
	RaDec	ephemeris(const JulianDate& when);
};

/**
 * \brief Consider the Moon a solar system body
 */
class Moon : public SolarsystemBody {
	virtual RaDec	ephemerisT(double T0);
public:
	Moon();
};

/**
 * \brief Consider the Sun a solar system body
 */
class Sun : public SolarsystemBody {
	virtual RaDec	ephemerisT(double T0);
public:
	Sun();
};

typedef std::shared_ptr<SolarsystemBody>	SolarsystemBodyPtr;

class	SolarsystemFactory {
public:
	static SolarsystemBodyPtr	get(const std::string& name);
};

/**
 * \brief A class used as arguments for time in planet position computations
 */
class	JulianCenturies {
	double	_T;
	double	setup(const JulianDate& when) const;
public:
	JulianCenturies(time_t t) : _T(setup(JulianDate(t))) { }
	JulianCenturies(const JulianDate& when) : _T(setup(when)) { }
	double	T() const { return _T; }
	operator double() const { return _T; }
};

/**
 * \brief Many of the computations are done in ecliptical coordinates
 */
class	EclipticalCoordinates {
	Angle	_l;
	double	_r;
	Angle	_b;
public:
	EclipticalCoordinates(const Angle& l, double r, const Angle& b);
	EclipticalCoordinates();
	const Angle&	l() const { return _l; }
	double	r() const { return _r; }
	const Angle&	b() const { return _b; }
	Vector	v() const;

	EclipticalCoordinates	operator+(const EclipticalCoordinates& other) const;
	EclipticalCoordinates	operator-(const EclipticalCoordinates& other) const;
	EclipticalCoordinates	operator*(double m) const;
	std::string	toString() const;
};

std::ostream&	operator<<(std::ostream& out, const EclipticalCoordinates& ec);

EclipticalCoordinates	operator*(double m, const EclipticalCoordinates& ecl);

/**
 * \brief Auxiliary class to streamline computation of cos(k*x) and sin(k*x)
 */
class	SinCos : public Angle {
	double	_cos;
	double	_sin;
public:
	SinCos(double cos, double sin);
	SinCos();
	SinCos(const Angle& a);
	SinCos(const std::pair<double,double>& p);
	double	sin() const { return _sin; }
	double	cos() const { return _cos; }
	SinCos	operator*(int k) const;
	SinCos	operator-() const;
	SinCos	operator+(const SinCos& other) const;
	SinCos	operator-(const SinCos& other) const;
};

/**
 * \brief A planetoid is the base class for all planetary objects
 */
class	Planetoid {
	std::string	_name;
	double	_a;		// semimajor axis
	double	_e;		// excentricity
	Angle	_Omega;		// ascending node
	Angle	_i;		// orbital inclination
	Angle	_omega;		// perihelion argument
	Angle	_n;		// anomaly
	Angle	_M0;		// perihelion angle
public:
	Planetoid(const std::string& name,
		double a, double e, const Angle& Omega, const Angle& i,
		const Angle& omega, const Angle& n, const Angle& M0);
	const std::string&	name() const { return _name; }
	double	a() const { return _a; }
	double	e() const { return _e; }
	const Angle&	Omega() const { return _Omega; }
	const Angle&	i() const { return _i; }
	const Angle&	omega() const { return _omega; }
	const Angle&	n() const { return _n; }
	const Angle&	M0() const { return _M0; }
	void	n(const Angle& n) { _n = n; }
	void	M0(const Angle& M0) { _M0 = M0; }
protected:
	Angle	l(const SinCos& m) const;
	Angle	b(const SinCos& m) const;
	double	r(const SinCos& m) const;
public:
	Angle	M(const JulianCenturies& T) const;
	SinCos	Msc(const JulianCenturies& T) const;
	EclipticalCoordinates	ecliptical(const JulianCenturies& T) const;
	Vector	XYZ(const JulianCenturies& T) const;
	std::string	toString(Angle::unit unit = Angle::Degrees) const;
};

std::ostream&	operator<<(std::ostream& out, const Planetoid& planetoid);

typedef std::shared_ptr<Planetoid>	PlanetoidPtr;

class	Mercury : public Planetoid {
public:
	Mercury();
};

class	Venus : public Planetoid {
public:
	Venus();
};

class	Earth : public Planetoid {
public:
	Earth();
};

class	Mars : public Planetoid {
public:
	Mars();
};

class	Jupiter : public Planetoid {
public:
	Jupiter();
};

class	Saturn : public Planetoid {
public:
	Saturn();
};

class	Uranus : public Planetoid {
public:
	Uranus();
};

class	Neptune : public Planetoid {
public:
	Neptune();
};

class	Pluto : public Planetoid {
public:
	Pluto();
};

/**
 * \brief Perturbation series Term
 */
class	PerturbationTerm {
	const Planetoid&	_perturbed;
	const Planetoid&	_perturber;
	int	_perturbed_i;
	int	_perturber_i;
	int	_T_exponent;
	Angle	_dl_cos;
	Angle	_dl_sin;
	double	_dr_cos;
	double	_dr_sin;
	Angle	_db_cos;
	Angle	_db_sin;
public:
	PerturbationTerm(const Planetoid& perturbed, const Planetoid& perturber,
		int perturbed_i, int perturber_i, int T_exponent,
		const Angle& dl_cos, const Angle& dl_sin,
		double dr_cos, double dr_sin,
		const Angle& db_cos, const Angle& db_sin);
	EclipticalCoordinates	operator()(const JulianCenturies& T) const;
};

typedef std::shared_ptr<PerturbationTerm>	PerturbationTermPtr;

/**
 * \brief A perturbation series
 */
class	PerturbationSeries : std::list<PerturbationTerm> {
	const Planetoid&	_perturbed;
	Planetoid	_perturber;
public:
	PerturbationSeries(const Planetoid& perturbed,
		const Planetoid& perturber);
	PerturbationTerm	add(int perturbed_i, int perturber_i,
				int T_exponent,
				const Angle& dl_cos, const Angle& dl_sin,
				double dr_cos, double dr_sin,
				const Angle& db_cos, const Angle& db_sin);
	PerturbationTerm	add(int perturbed_i, int perturber_i,
				int T_exponent,
				double dl_cos, double dl_sin,
				double dr_cos, double dr_sin,
				double db_cos, double db_sin);
	EclipticalCoordinates	operator()(const JulianCenturies& T) const;
	EclipticalCoordinates	perturbations(const JulianCenturies& T) const;
	Planetoid&	perturber() { return _perturber; }
};

typedef std::shared_ptr<PerturbationSeries>	PerturbationSeriesPtr;

/**
 * \brief A planetoid with a perturbation series attached
 */
class	PerturbedPlanetoid : public Planetoid {
	std::list<PerturbationSeriesPtr>	_perturbers;
public:
	PerturbedPlanetoid(const std::string& name,
		double a, double e, const Angle& Omega, const Angle& i,
		const Angle& omega, const Angle& n, const Angle& M0);
	PerturbedPlanetoid(const Planetoid& p);
	void	add(PerturbationSeriesPtr series);
	EclipticalCoordinates	perturbations(const JulianCenturies& T) const;
	EclipticalCoordinates	ecliptical(const JulianCenturies& T) const;
};

class	MercuryPerturbed : public PerturbedPlanetoid {
	PerturbationSeriesPtr	venus;
	PerturbationSeriesPtr	earth;
	PerturbationSeriesPtr	jupiter;
	PerturbationSeriesPtr	saturn;
public:
	MercuryPerturbed();
};

class	VenusPerturbed : public PerturbedPlanetoid {
	PerturbationSeriesPtr	mercury;
	PerturbationSeriesPtr	earth;
	PerturbationSeriesPtr	mars;
	PerturbationSeriesPtr	jupiter;
	PerturbationSeriesPtr	saturn;
public:
	VenusPerturbed();
};

class	EarthPerturbed : public PerturbedPlanetoid {
public:
	EarthPerturbed();
};

class	MarsPerturbed : public PerturbedPlanetoid {
	PerturbationSeriesPtr	venus;
	PerturbationSeriesPtr	earth;
	PerturbationSeriesPtr	jupiter;
	PerturbationSeriesPtr	saturn;
public:
	MarsPerturbed();
};

class	JupiterPerturbed : public PerturbedPlanetoid {
	PerturbationSeriesPtr	saturn;
	PerturbationSeriesPtr	uranus;
public:
	JupiterPerturbed();
};

class	SaturnPerturbed : public PerturbedPlanetoid {
	PerturbationSeriesPtr	jupiter;
	PerturbationSeriesPtr	uranus;
	PerturbationSeriesPtr	neptune;
public:
	SaturnPerturbed();
};

class	UranusPerturbed : public PerturbedPlanetoid {
	PerturbationSeriesPtr	jupiter;
	PerturbationSeriesPtr	saturn;
	PerturbationSeriesPtr	neptune;
public:
	UranusPerturbed();
};

class	NeptunePerturbed : public PerturbedPlanetoid {
	PerturbationSeriesPtr	jupiter;
	PerturbationSeriesPtr	saturn;
	PerturbationSeriesPtr	uranus;
public:
	NeptunePerturbed();
};

class	PlutoPerturbed : public PerturbedPlanetoid {
	PerturbationSeriesPtr	jupiter;
	PerturbationSeriesPtr	saturn;
public:
	PlutoPerturbed();
};


} // namespace solarsystem
} // namespace astro

#endif /* _AstroSolarsystem_h */
