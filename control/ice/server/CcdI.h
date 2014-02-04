/*
 * CcdI.h -- ICE CCD wrapper class definition
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _CcdI_h
#define _CcdI_h

#include <camera.h>
#include <AstroCamera.h>
#include <ImageDirectory.h>

namespace snowstar {

class CcdI : public Ccd {
	astro::camera::CcdPtr	_ccd;
	astro::image::ImageDirectory	_imagedirectory;
	time_t	laststart;
	astro::image::ImagePtr	image;
public:
	CcdI(astro::camera::CcdPtr ccd,
		astro::image::ImageDirectory& imagedirectory)
			: _ccd(ccd), _imagedirectory(imagedirectory) { }
	virtual	~CcdI();

static	CcdPrx	createProxy(const std::string& ccdname, const Ice::Current& current);

	// interface methods
	std::string	getName(const Ice::Current& current);
	CcdInfo	getInfo(const Ice::Current& current);
	void	startExposure(const Exposure&, const Ice::Current& current);
	ExposureState	exposureStatus(const Ice::Current& current);
	int	lastExposureStart(const Ice::Current& current);
	void	cancelExposure(const Ice::Current& current);
	Exposure	getExposure(const Ice::Current& current);
	ImagePrx	getImage(const Ice::Current& current);
	bool	hasGain(const Ice::Current& current);
	bool	hasShutter(const Ice::Current& current);
	ShutterState	getShutterState(const Ice::Current& current);
	void	setShutterState(ShutterState, const Ice::Current& current);
	bool	hasCooler(const Ice::Current& current);
	CoolerPrx	getCooler(const Ice::Current& current);
};

} // namespace snowstar

#endif /* _CcdI_h */
