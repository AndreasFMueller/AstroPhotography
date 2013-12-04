/*
 * TaskParameters.cpp -- Implementation of the TaskParameters class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTask.h>
#include <AstroDebug.h>

namespace astro {
namespace task {

TaskParameters::TaskParameters() {
	_ccdid = 0;
	_filterposition = 0;
	_ccdtemperature = -1;
}

} // namespace task
} // namespace astro
