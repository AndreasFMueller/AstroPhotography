/*
 * SxCcd.cpp -- Starlight express CCD implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SxCcd.h>
#include <AstroCamera.h>
#include <AstroImage.h>
#include <sx.h>

using namespace astro::camera;

namespace astro {
namespace camera {
namespace sx {

SxCcd::SxCcd(const CcdInfo& info, SxCamera& _camera, int _ccdindex)
	: Ccd(info), camera(_camera), ccdindex(_ccdindex) {
}

SxCcd::~SxCcd() {
}

Exposure::State	SxCcd::exposureStatus() throw (not_implemented) {
	return state;
}

void	SxCcd::startExposure(const Exposure& exposure) throw (not_implemented) {
	// remember the exposure
	this->exposure = exposure;

	// create the exposure request
	sx_read_pixels_delayed_t	rpd;
	rpd.x_offset = exposure.frame.origin.x;
	rpd.y_offset = exposure.frame.origin.y;
	rpd.width = exposure.frame.size.width;
	rpd.height = exposure.frame.size.height;
	rpd.x_bin = exposure.mode.getX();
	rpd.x_bin = exposure.mode.getY();
	rpd.delay = 1000 * exposure.exposuretime;

	// build a control request
	Request<sx_read_pixels_delayed_t>	request(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, ccdindex,
		(uint8_t)SX_CMD_READ_PIXELS_DELAYED, (uint16_t)0, &rpd);
	camera.getDevicePtr()->controlRequest(&request);

	// we are now in exposing state
	state = Exposure::exposing;
}

ShortImagePtr	SxCcd::shortImage() throw (not_implemented) {
	// compute the size of the buffer, and create a buffer for the
	// data
	int	size = 2 * exposure.frame.size.pixels;
	unsigned short	*data = new unsigned short[size];

	// find the interface
	ConfigurationPtr	config = camera.getDevicePtr()->activeConfig();
	InterfacePtr	interface = (*config)[0];
	interface->claim();

	// get the endpoint
	InterfaceDescriptorPtr	idptr = (*interface)[0];
	EndpointDescriptorPtr	endpoint = (*idptr)[2];

	// read the data from the data endpoint
	BulkTransfer	transfer(endpoint, size, (unsigned char *)data);

	// submit the transfer
	camera.getDevicePtr()->submit(&transfer);

	// release the interface again
	interface->release();

	// when the transfer completes, one can use the data for the
	// image
	Image<unsigned short>	*image
		= new Image<unsigned short>(exposure.frame.size, data);
	return ShortImagePtr(image);
}

SxCcdM26C::SxCcdM26C(const CcdInfo& info, SxCamera& camera, int id)
	: SxCcd(info, camera, id) {
}

void	SxCcdM26C::startExposure(const Exposure& exposure)
		throw (not_implemented) {
	// remember the exposre, we need it for the second field for the
	// case where we do two fields one after the other
	this->exposure = exposure;

	// compute a better request for the M26C camera
	sx_read_pixels_delayed_t	rpd;

	// send the request to the camera
	Request<sx_read_pixels_delayed_t>	request(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, ccdindex,
		(uint8_t)SX_CMD_READ_PIXELS_DELAYED, (uint16_t)0, &rpd);
	camera.getDevicePtr()->controlRequest(&request);

	// we are now in exposing state
	state = Exposure::exposing;
}

ShortImagePtr	SxCcdM26C::shortImage() throw (not_implemented) {
	return ShortImagePtr();
}

SxCcdM26C::~SxCcdM26C() {
}

} // namespace sx
} // namespace camera
} // namespace astro
