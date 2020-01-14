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

} // namespace solarsystem
} // namespace astro

#endif /* _AstroSolarsystem_h */
