/*
 * Ccd.cpp -- Ccd implementation
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroCamera.h>

namespace astro {
namespace camera {

/**
 * \brief Start an exposure
 *
 * Override this function to initiate an exposure. This function
 * should return immediately. The caller can then use the exposureStatus
 * method to monitor the progress of the exposure.
 */
void    Ccd::startExposure(const Exposure& exposure) throw (not_implemented) {
	throw not_implemented("startExposureStatus not implemented");
}

/**
 * \brief Monitor progress of an exposure
 *
 * Find out whether an exposure is in progress. Optional method.
 */
Exposure::State Ccd::exposureStatus() throw (not_implemented) {
	throw not_implemented("exposureStatus not implemented");
}

/**
 * \brief Cancel an exposure
 *
 * Note that some cameras cannot cancel an exposure other than by
 * resetting the camera, which will affect other CCDs of the same
 * camera as well. If you plan to implement this function for such
 * a camera,
 * make sure that you would usually read from the camera is also
 * stored locally so that it can be restored after the reset.
 */
void    Ccd::cancelExposure() throw (not_implemented) {
	throw not_implemented("cancelExposure not implemented");
}

/**
 * \brief Retrieve a 8bit raw image from the camera
 */
astro::image::ByteImage       Ccd::byteImage() throw (not_implemented) {
	throw not_implemented("byteImage not implemented");
}

/**
 * \brief Retrieve a 16bit raw image from the camera
 */
astro::image::ShortImage      Ccd::shortImage() throw (not_implemented) {
	throw not_implemented("shortImage not implemented");
}

/**
 * \brief Retrieve an YUYV image from the camera
 */
astro::image::YuyvImage       Ccd::yuyvImage() throw (not_implemented) {
	throw not_implemented("yuyvImage not implemented");
}

/**
 * \brief Retrieve an YUYV image from the camera
 */
astro::image::RgbImage        Ccd::rgbImage() throw (not_implemented) {
	throw not_implemented("rgbImage not implemented");
}

} // namespace camera
} // namespace astro
