/*
 * SxCcd.h -- abstraction for the CCD of a starlight express camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SxCcd_h
#define _SxCcd_h

#include "SxCamera.h"
#include "SxDemux.h"
#include <AstroImage.h>
#include <AstroUtils.h>
#include <thread>

using namespace astro::camera;
using namespace astro::image;

namespace astro {
namespace camera {
namespace sx {

/**
 * \brief Basic Starlight Express class
 *
 * Most Starlight Express cameras have a progressive CCD and are relatively
 * simple to control. Only the M26C is an exception, we use a derived class 
 * to handle these differences
 */
class SxCcd : public Ccd {
	// we need a separate thread that retrieves the image
	std::thread	thread;
protected:
	ImagePtr	image;
	SxCamera&	camera;
	int	ccdindex;
private:
	SxCcd(const SxCcd&);
	SxCcd&	operator=(const SxCcd&);
public:
	SxCcd(const CcdInfo& info, SxCamera& camera, int ccdindex);
	virtual ~SxCcd();
protected:
	virtual void	startExposure0(const Exposure& exposure);
public:
	virtual void	startExposure(const Exposure& exposure);
	virtual void	getImage0();
public:
	virtual ImagePtr	getRawImage();

	// cooler stuff
	virtual bool	hasCooler() const;
protected:
	virtual CoolerPtr	getCooler0();
};

/**
 * \brief Special class for the imaging CCD of the M26C camera.
 *
 * The M26C camera has a completely different CCD, so much so that a separate
 * implementation is indicated.
 */
class SxCcdM26C : public SxCcd {
	Exposure	m26cExposure();
	Exposure	symmetrize(const Exposure& exp) const;
	Exposure	m26c;
	Timer	timer;
	void	exposeField(int field);
	void	requestField(int field);
	Field 	*readField();
public:
	SxCcdM26C(const CcdInfo& info, SxCamera& camera, int ccdindex);
	virtual ~SxCcdM26C();
	virtual void	startExposure0(const Exposure& exposure);
	virtual void	getImage0();
};

} // namespace sx
} // namespace camera
} // namespace astro

#endif /* _SxCcd_h */
