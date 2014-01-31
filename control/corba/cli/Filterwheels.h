/*
 * Filterwheels.h -- filterwheel reference repository
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Filterwheels_h
#define _Filterwheels_h

#include <ObjWrapper.h>
#include <camera.hh>
#include <vector>

namespace astro {
namespace cli {

typedef ObjWrapper<Astro::FilterWheel>	FilterwheelWrapper;

class Filterwheel_internals;

/**
 * \brief class to mediate access to ccd references by short name
 */
class Filterwheels {
	static Filterwheel_internals *internals;
public:
	Filterwheels();
	FilterwheelWrapper	byname(const std::string& filterwheelid);
	void	release(const std::string& filterwheelid);
	void	assign(const std::string& filterwheelid,
			const std::vector<std::string>& arguments);
};

} // namespace cli
} // namespace astro

#endif /* _Filterwheels_h */
