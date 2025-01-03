/*
 * utils.cpp -- utilities for the asi driver implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <utils.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <AstroDebug.h>
#include <AstroFormat.h>
#include <includes.h>

namespace astro {
namespace camera {
namespace asi {

//////////////////////////////////////////////////////////////////////
// AsiLocator implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief auxiliary function to generate camera name
 *
 * The name of an SBIG camera is essentially the serial number of the camera
 */
DeviceName	asiCameraName(int index) {
	std::string	name = stringprintf("%d", index);
	DeviceName	cameraname(DeviceName::Camera, "asi", name);
	return cameraname;
}

/**
 * \brief Generate a CCD name from the camera
 *
 * The name generated is designed to work with the implementation of the
 * getGuideport0 function in the base DeviceLocator class, so that no
 * ASI-specific implementation of this function is required.
 */
DeviceName	asiCcdName(int index, const std::string& imgtype) {
	DeviceName	cameraname = asiCameraName(index);
	DeviceName	ccdname = cameraname.child(DeviceName::Ccd, imgtype);
	return ccdname;
}

/**
 * \brief Generate the name of a cooler for an ASI Cooler camera
 */
DeviceName	asiCoolerName(int index) {
	DeviceName	coolername = asiCameraName(index);
	coolername.type(DeviceName::Cooler);
	return coolername;
}

/**
 * \brief Generate a guider port name from the camera
 *
 * The name generated is designed to work with the implementation of the
 * getGuideport0 function in the base DeviceLocator class, so that no
 * ASI-specific implementation of this function is required.
 */
DeviceName	asiGuideportName(int index) {
	DeviceName	guideportname = asiCameraName(index);
	guideportname.type(DeviceName::Guideport);
	return guideportname;
}


} // namespace asi
} // namespace camera
} // namespace astro
