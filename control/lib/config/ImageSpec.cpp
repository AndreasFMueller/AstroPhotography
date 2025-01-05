/*
 * ImageSpec.cpp
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProject.h>

namespace astro {
namespace project {

ImageSpec::ImageSpec() {
	//_purpose = astro::camera::Exposure::light;
	_purpose = (astro::camera::Exposure::purpose_t)-1;
	_exposuretime = -1;
	_gain = -1;
	_temperature = -300;
}

} // namespace project
} // namespace astro
