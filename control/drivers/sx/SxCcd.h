/*
 * SxCcd.h -- abstraction for the CCD of a starlight express camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SxCcd_h
#define _SxCcd_h

#include <SxCamera.h>
#include <AstroImage.h>
#include <AstroUtils.h>

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
protected:
	SxCamera&	camera;
	int	ccdindex;
public:
	SxCcd(const CcdInfo& info, SxCamera& camera, int ccdindex);
	virtual ~SxCcd();
	virtual Exposure::State	exposureStatus() throw (not_implemented);
	virtual void	startExposure(const Exposure& exposure)
		throw (not_implemented);
	virtual ShortImagePtr	shortImage() throw (not_implemented);
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
	class	Field {
	public:
		size_t	length;
		unsigned short	*data;
		Field(size_t l);
		~Field();
		void	rescale(double scale);
	};
	Field 	*readField();
public:
	SxCcdM26C(const CcdInfo& info, SxCamera& camera, int ccdindex);
	virtual ~SxCcdM26C();
	virtual void	startExposure(const Exposure& exposure)
		throw (not_implemented);
	virtual ShortImagePtr	shortImage() throw (not_implemented);
};

} // namespace sx
} // namespace camera
} // namespace astro

#endif /* _SxCcd_h */
