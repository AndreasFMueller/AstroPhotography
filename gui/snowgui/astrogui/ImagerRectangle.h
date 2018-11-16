/*
 * ImagerRectangle.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _ImagerRectangle_h
#define _ImagerRectangle_h

#include <AstroCoordinates.h>

namespace snowgui {

class ImagerRectangle {
	astro::Angle		_azimuth;
	astro::TwoAngles	_size;
public:
	const astro::Angle&	azimuth() const { return _azimuth; }
	void	azimuth(const astro::Angle& a) { _azimuth = a; }
	const astro::TwoAngles&	size() const { return _size; }
	void	size(const astro::TwoAngles& s) { _size = s; }

	astro::RaDec	point(float x, float y) const;
	std::string	toString() const;
};

} // namespace snowgui

#endif /* _ImagerRectangle_h */
