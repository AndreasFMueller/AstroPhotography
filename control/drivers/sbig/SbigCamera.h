/*
 * SbigCamera.h -- SBIG camera abstraction
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SbigCamera_h
#define _SbigCamera_h

#include <includes.h>
#include <AstroCamera.h>

#ifdef HAVE_LPARDRV_H
#include <lpardrv.h>
#else
#ifdef HAVE_SBIGUDRV_LPARDRV_H
#include <SBIGUDrv/lpardrv.h>
#endif /* HAVE_SBIGUDRV_LPARDRV_H */
#endif

using namespace astro::camera;

namespace astro {
namespace camera {
namespace sbig {

class SbigDevice;
class SbigCcd;
class SbigFilterWheel;
class SbigCooler;
class SbigGuidePort;

/**
 * \brief SBIG camera object
 */
class SbigCamera : public Camera {
	unsigned short	cameraType;
	static short	current_handle;
	short	handle;
	void	sethandle();
	// encapsulated Driver Library calls
	void	query_usb(QueryUSBResults *results);
	void	open_device(int usbno);
	void	close_device();
	unsigned short	get_camera_type();
	short	get_driver_handle();
	CcdInfo	get_ccd_info(unsigned short request, const std::string& name,
			unsigned int ccdindex);
public:
	SbigCamera(int usbno);
	virtual ~SbigCamera();
protected:
	virtual CcdPtr	getCcd0(size_t id);
public:
	virtual bool	hasFilterWheel() const;
protected:
	virtual FilterWheelPtr	getFilterWheel0();
public:
	virtual bool	hasGuidePort() const;
protected:
	virtual GuidePortPtr	getGuidePort0();
	friend class SbigDevice;
	friend class SbigCcd;
	friend class SbigFilterWheel;
	friend class SbigCooler;
	friend class SbigGuidePort;
};

} // namespace sbig
} // namespace camera
} // namespace astro

#endif /* _SbigCamera_h */
