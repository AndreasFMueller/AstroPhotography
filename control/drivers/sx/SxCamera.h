	/*
 * SxCamera.h -- starlight express camera declarations
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SxCamera_h
#define _SxCamera_h

#include <AstroCamera.h>
#include <AstroUSB.h>
#include <SxLocator.h>
#include "sx.h"
#include <mutex>

using namespace astro::camera;
using namespace astro::usb;

namespace astro {
namespace camera {
namespace sx {

class SxCcd;
class SxCooler;
class SxGuidePort;

/**
 * \brief Starlight Xpress camera class
 *
 * The starlight express camera class 
 */
class SxCamera : public Camera {
	SxCameraLocator&	_locator;
	DevicePtr	deviceptr;
	uint16_t	model;
	uint16_t	product;
	InterfacePtr	interface;
	EndpointDescriptorPtr	outendpoint;
	EndpointDescriptorPtr	inendpoint;
	bool	useControlRequests;
	sx_firmware_version_t	firmware_version;
	int	build_number;
	bool	_hasCooler;
	bool	_hasGuidePort;
	bool	_has_interline_ccd;
public:
	bool	hasInterlineCcd() const { return _has_interline_ccd; }
	bool	hasRBIFlood() const;

	static unsigned short	getModel(DevicePtr deviceptr);

private:
	// a lock to ensure that only one USB operation at a time goes to
	// the camera
	std::recursive_mutex		mutex;
	std::condition_variable_any	condition;
	// stuff needed for reservation
	bool			_busy;
	std::string		_purpose;
public:
	bool	busy();
	std::string	purpose();
	bool	reserve(const std::string& purpose, int timeout = 1000);
	void	release(const std::string& purpose);
	void	refresh();

public:
	// USB related methods
	DevicePtr	getDevicePtr();
	EndpointDescriptorPtr	getEndpoint();
	InterfacePtr	getInterface();
private:
	void	disconnect();
	void	connect(DevicePtr devptr);
	void	reconnect(DevicePtr devptr);
public:
	// constructors
	SxCamera(SxCameraLocator& _locator, DevicePtr devptr);
	virtual ~SxCamera();

	virtual std::string	userFriendlyName() const;

	// reset
	virtual void	reset();

	// ccd access
protected:
	virtual CcdPtr	getCcd0(size_t id);
private:
	CoolerPtr	_cooler;
public:
	// cooler access
	bool	hasCooler();
	virtual CoolerPtr	getCooler(int ccdindex);

	// guider port access
public:
	virtual bool	hasGuidePort() const { return _hasGuidePort; }
private:
	GuidePortPtr	_guideport;
protected:
	virtual GuidePortPtr	getGuidePort0();

	// request handling
public:
	void	controlRequest(RequestBase *request,
		bool asUSBControlRequest = DEFAULT_AS_USB_CONTROL_REQUEST);
	static void	controlRequest(DevicePtr deviceptr,
				RequestBase *request);

	// find out whether this is a color camera
	bool	isColor() const;
};


} // namespace sx
} // namespace camera
} // namespace astro

#endif /* _SxCamera_h */
