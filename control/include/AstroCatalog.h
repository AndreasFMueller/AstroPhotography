/*
 * AstroCatalog.h -- Generic star catalog classes
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroCatalog_h
#define _AstroCatalog_h

#include <AstroCoordinates.h>
#include <set>
#include <limits.h>
#include <string>

namespace astro {
namespace catalog {

/**
 * \brief Window on the sky, used to select stars from the catalog
 */
class SkyWindow {
	RaDec	_center;
public:
	const RaDec&	center() const { return _center; }
private:
	Angle	_rawidth;
	Angle	_decheight;
public:
	const Angle&	rawidth() const { return _rawidth; }
	const Angle&	decheight() const { return _decheight; }
public:
	SkyWindow(const RaDec& center, const Angle& rawidth,
		const Angle& decheight);
	bool	contains(const RaDec& position) const;
	std::pair<double, double>	decinterval() const;
	Angle	leftra() const;
	Angle	rightra() const;
	virtual std::string	toString() const;
};

/**
 * \brief Star base class
 */
class Star : public RaDec {
protected:
	float	_mag;
public:
	const float&	mag() const { return _mag; }
	float&	mag() { return _mag; }
	Star() { _mag = 0; }
	std::string	toString() const;
};

} // namespace catalog
} // namespace astro

#endif /* _AstroCatalog_h */
