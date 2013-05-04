/*
 * Ccd.cpp -- Ccd implementation
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroCamera.h>
#include <Format.h>

namespace astro {
namespace camera {

//////////////////////////////////////////////////////////////////////
// CcdInfo implementation
//////////////////////////////////////////////////////////////////////

CcdInfo::CcdInfo() {
}

const astro::image::ImageSize&	CcdInfo::getSize() const {
	return size;
}

const BinningSet&	CcdInfo::modes() const {
	return binningmodes;
}

const std::string&	CcdInfo::getName() const {
	return name;
}

int	CcdInfo::getId() const {
	return ccdid;
}

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
 * \brief Retrieve a 8bit raw image from the camera
 */
astro::image::ByteImagePtr	Ccd::byteImage() throw (not_implemented) {
	throw not_implemented("byteImage not implemented");
}

/**
 * \brief Retrieve a 16bit raw image from the camera
 */
astro::image::ShortImagePtr	Ccd::shortImage() throw (not_implemented) {
	throw not_implemented("shortImage not implemented");
}

/**
 * \brief Retrieve a 32bit raw image from the camera
 */
astro::image::IntImagePtr	Ccd::intImage() throw (not_implemented) {
	throw not_implemented("intImage not implemented");
}

/**
 * \brief Retrieve a 16bit raw image from the camera
 */
astro::image::LongImagePtr	Ccd::longImage() throw (not_implemented) {
	throw not_implemented("longImage not implemented");
}

/**
 * \brief Retrieve an YUYV image from the camera
 */
astro::image::YUYVImagePtr	Ccd::yuyvImage() throw (not_implemented) {
	throw not_implemented("yuyvImage not implemented");
}

/**
 * \brief Retrieve an YUYV image from the camera
 */
astro::image::RGBImagePtr	Ccd::rgbImage() throw (not_implemented) {
	throw not_implemented("rgbImage not implemented");
}

/**
 * \brief Retrieve a cooler
 */
CoolerPtr	Ccd::getCooler() throw (not_implemented) {
	throw not_implemented("thermoelectric cooler not implemented");
}

} // namespace camera
} // namespace astro
