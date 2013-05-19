/*
 * SxCcd.h -- abstraction for the CCD of a starlight express camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SxCcd_h
#define _SxCcd_h

#include <SxCamera.h>
#include <SxDemux.h>
#include <AstroImage.h>
#include <AstroUtils.h>

using namespace astro::camera;
using namespace astro::image;

namespace astro {
namespace camera {
namespace sx {

/**
 * \brief Starlight Express Cooler abstraction
 *
 * The Starlight Express cameras do have a cooler and a proprietary API,
 * this class encapsulates that.
 */
class SxCooler : public Cooler {
	SxCamera&	camera;
	bool	cooler_on;
	float	settemperature;
	float	actualtemperature;
	void	cmd();
public:
	SxCooler(SxCamera& camera);
	virtual	~SxCooler();
	//virtual float	getSetTemperature();
	virtual float	getActualTemperature();
	virtual void	setTemperature(float temperature);
	virtual bool	isOn();
	virtual	void	setOn(bool onoff);
};

/**
 * \brief Basic Starlight Express class
 *
 * Most Starlight Express cameras have a progressive CCD and are relatively
 * simple to control. Only the M26C is an exception, we use a derived class 
 * to handle these differences
 */
class SxCcd : public Ccd {
protected:
	SxCamera&	camera;
	int	ccdindex;
public:
	SxCcd(const CcdInfo& info, SxCamera& camera, int ccdindex);
	virtual ~SxCcd();
	virtual Exposure::State	exposureStatus() throw (not_implemented);
	virtual void	startExposure(const Exposure& exposure)
		throw (not_implemented);
	virtual ImagePtr	getImage() throw (not_implemented);
	virtual CoolerPtr	getCooler() throw (not_implemented);
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
	virtual void	startExposure(const Exposure& exposure)
		throw (not_implemented);
	virtual ImagePtr	getImage() throw (not_implemented);
};

} // namespace sx
} // namespace camera
} // namespace astro

#endif /* _SxCcd_h */
