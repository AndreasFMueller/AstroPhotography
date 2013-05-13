/*
 * Ccd.cpp -- Ccd implementation
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroCamera.h>
#include <Format.h>
#include <debug.h>

using namespace astro::image;

namespace astro {
namespace camera {

//////////////////////////////////////////////////////////////////////
// CcdInfo implementation
//////////////////////////////////////////////////////////////////////

CcdInfo::CcdInfo() {
}

/**
 * \brief Get the CCD size.
 */
const astro::image::ImageSize&	CcdInfo::getSize() const {
	return size;
}

/**
 * \brief Get a frame filling the CCD
 *
 * This method returns an image rectangle that fills the CCD. This can
 * be used to initialize the exposure object for the getExposure
 * method. Some cameras, like the UVC cameras, can only display full
 * frames, not subframes.
 */
const ImageRectangle	CcdInfo::getFrame() const {
	return ImageRectangle(ImagePoint(0, 0), size);
}

/**
 * \brief Get the binning modes available for this CCD.
 */
const BinningSet&	CcdInfo::modes() const {
	return binningmodes;
}

/**
 * \brief Get the name of this CCD.
 */
const std::string&	CcdInfo::getName() const {
	return name;
}

/**
 * \brief Get the CCD id for this CCD.
 *
 * This is the index into the array of CCDs this camera has.
 */
int	CcdInfo::getId() const {
	return ccdid;
}

/**
 * \brief Return a string representation.
 */
std::string	CcdInfo::toString() const {
	return stringprintf("%s: %dx%d,%s", name.c_str(),
		size.width, size.height,
		binningmodes.toString().c_str());
}

std::ostream&	operator<<(std::ostream& out, const CcdInfo& ccdinfo) {
	return out << ccdinfo.toString();
}

//////////////////////////////////////////////////////////////////////
// Ccd implementation
//////////////////////////////////////////////////////////////////////

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
 * \brief Retrieve an image from the camera
 */
astro::image::ImagePtr	Ccd::getImage() throw (not_implemented) {
	throw not_implemented("getImage not implemented");
}

/**
 * \brief Retrieve a sequence of images from the camera
 *
 * The default implementation just performs multiple startExposure/getImage
 * calls. We reuse the same exposure structure for all calls.
 * \param imagecount	number of images to retrieve
 */
astro::image::ImageSequence	Ccd::getImageSequence(unsigned int imagecount)
	throw (not_implemented) {
	astro::image::ImageSequence	result;
	unsigned int	k = 0;
	while (k < imagecount) {
		if (k > 0) {
			startExposure(exposure);
		}
		result.push_back(getImage());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image %d retrieved", k);
		k++;
	}
	return result;
}

/**
 * \brief Retrieve a cooler
 */
CoolerPtr	Ccd::getCooler() throw (not_implemented) {
	throw not_implemented("thermoelectric cooler not implemented");
}

} // namespace camera
} // namespace astro
