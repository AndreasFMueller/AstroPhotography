/*
 * UvcCcd.h -- ccd implementation for the UVC driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <UvcCamera.h>

using namespace astro::image;

namespace astro {
namespace camera {
namespace uvc {

class UvcCcd : public Ccd {
	int	interface;
	int	format;
	int	frame;
	UvcCamera&	camera;
	virtual ImagePtr	frameToImage(const Frame& frame) const = 0;
public:
	UvcCcd(const CcdInfo& info, int interface, int format, int frame,
		UvcCamera& camera);
	virtual void    startExposure(const Exposure& exposure);
	virtual ImagePtr	getRawImage();
	virtual ImageSequence	getImageSequence(unsigned int imagecount);

	// gain control
	virtual bool	hasGain();
	virtual void	setGain(double gain);
	virtual std::pair<float, float>	gainInterval();
};

class UvcCcdYUY2 : public UvcCcd {
	virtual ImagePtr	frameToImage(const Frame& frame) const;
public:
	UvcCcdYUY2(const CcdInfo& info, int interface, int format, int frame,
		UvcCamera& camera);
};

class UvcCcdY800 : public UvcCcd {
	virtual ImagePtr	frameToImage(const Frame& frame) const;
public:
	UvcCcdY800(const CcdInfo& info, int interface, int format, int frame,
		UvcCamera& camera);
};

class UvcCcdBY8 : public UvcCcd {
	virtual ImagePtr	frameToImage(const Frame& frame) const;
public:
	UvcCcdBY8(const CcdInfo& info, int interface, int format, int frame,
		UvcCamera& camera);
};


} // namespace uvc
} // namespace camera
} // namespace astro
