/*
 * SxCamera.h -- starlight express camera declarations
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SxCamera_h
#define _SxCamera_h

#include <AstroCamera.h>
#include <AstroUSB.h>
#include <sx.h>

using namespace astro::camera;
using namespace astro::usb;

namespace astro {
namespace camera {
namespace sx {

/**
 * \brief Starlight Xpress camera class
 *
 * The starlight express camera class 
 */
class SxCamera : public Camera {
	DevicePtr	deviceptr;
	uint16_t	model;
	uint16_t	product;
	InterfacePtr	interface;
	EndpointDescriptorPtr	outendpoint;
	EndpointDescriptorPtr	inendpoint;
	bool	useControlRequests;
	sx_firmware_version_t	firmware_version;
	bool	_hasCooler;
	bool	_hasGuiderPort;
	bool	_has_interline_ccd;
public:
	bool	hasInterlineCcd() const { return _has_interline_ccd; }
public:
	// USB related methods
	DevicePtr	getDevicePtr();
	EndpointDescriptorPtr	getEndpoint();
	InterfacePtr	getInterface();

	// constructors
	SxCamera(DevicePtr& devptr);
	virtual ~SxCamera();

	// reset
	virtual void	reset();

	// ccd access
protected:
	virtual CcdPtr	getCcd0(size_t id);

public:
	// cooler access
	bool	hasCooler();
	virtual CoolerPtr	getCooler(int ccdindex);

	// guider port access
public:
	virtual bool	hasGuiderPort() const { return _hasGuiderPort; }
protected:
	virtual GuiderPortPtr	getGuiderPort0();

	// request handling
public:
	void	controlRequest(RequestBase *request);

	// find out whether this is a color camera
	bool	isColor() const;
};


} // namespace sx
} // namespace camera
} // namespace astro

#endif /* _SxCamera_h */
