/*
 * ImageSpec.cpp
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProject.h>

namespace astro {
namespace project {

ImageSpec::ImageSpec() {
	_category = light;
	_exposuretime = -1;
	_temperature = -300;
}

} // namespace project
} // namespace astro
