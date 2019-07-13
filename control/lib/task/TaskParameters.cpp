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
	_ccdtemperature = -1;
	_cameraindex = -1;
	_ccdindex = -1;
	_coolerindex = -1;
	_filterwheelindex = -1;
	_mountindex = -1;
	_focuserindex = -1;
	_guiderccdindex = -1;
	_guideportindex = -1;
	_adaptiveopticsindex = -1;
}

} // namespace task
} // namespace astro
