/*
 * NetCcd.cpp -- network based CCD implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NetCcd.h>
#include <NetCooler.h>
#include <Conversions.h>
#include <includes.h>
#include <stdexcept>
#include <AstroIO.h>

namespace astro {
namespace camera {
namespace net {

NetCcd::NetCcd(const CcdInfo& _info, Astro::Ccd_ptr ccd)
	: Ccd(_info), _ccd(ccd) {
	Astro::Ccd_Helper::duplicate(_ccd);
	// an exposure may already be in progress, so we make sure we 
	// retrieve the remote state
	state = convert(_ccd->exposureStatus());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remote exposure state: %s",
		convert2string(state).c_str());
	exposure = convert(_ccd->getExposure());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remote exposure parameters: %s",
		exposure.toString().c_str());
}

NetCcd::~NetCcd() {
	Astro::Ccd_Helper::release(_ccd);
}

void	NetCcd::startExposure(const Exposure& _exposure) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start a new exposure");
	Ccd::startExposure(_exposure);
	_ccd->startExposure(convert(_exposure));
	exposure = convert(_ccd->getExposure());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure returned from remote camera:"
		" %s", exposure.toString().c_str());
	state = convert(_ccd->exposureStatus());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure status now %s",
		convert2string(state).c_str());
}

Exposure::State	NetCcd::exposureStatus() {
	return convert(_ccd->exposureStatus());
}

void	NetCcd::cancelExposure() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cancelling exposure");
	_ccd->cancelExposure();
	state = convert(_ccd->exposureStatus());
}

/**
 * \brief Get the image
 *
 * This method is somewhat convoluted, because we have to go through the
 * file system to convert the data into an Image object
 */
ImagePtr	NetCcd::getImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve image");
	Astro::Image_ptr	image = _ccd->getImage();

	Astro::Image::ImageFile	*imagefile = image->file();

	// create a filename where we want to place the file data
	char	filename[1024];
	const char	*tempdir = getenv("TMPDIR");
	if (NULL == tempdir) {
		tempdir = "/tmp";
	}
	snprintf(filename, sizeof(filename), "%s/netXXXXXX.fits", tempdir);
	mkstemps(filename, 5);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "temporary filename: %s", filename);

	// write the data to the file
	int	fd = open(filename, O_WRONLY);
	if (fd < 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"cannot open '%s' for writing: %s", filename,
			strerror(errno));
		throw std::runtime_error("cannot open file");
	}

	// write the data to the file
	write(fd, imagefile->get_buffer(), imagefile->length());

	// close the file
	close(fd);

	// read the file data again
	astro::io::FITSin	in(filename);
	ImagePtr	_image = in.read();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "file '%s' written", filename);

	// remove the file
	unlink(filename);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "temporary file '%s' deleted", filename);

	return _image;
}

bool	NetCcd::hasCooler() const {
	return _ccd->hasCooler();
}

CoolerPtr	NetCcd::getCooler0() {
	Astro::Cooler_var	cooler = _ccd->getCooler();
	return CoolerPtr(new NetCooler(cooler));
}

shutter_state	NetCcd::getShutterState() {
	return convert(_ccd->getShutterState());
}

void	NetCcd::setShutterState(const shutter_state& state) {
	_ccd->setShutterState(convert(state));
}

} // namespace net
} // namespace camera
} // namespace astro
