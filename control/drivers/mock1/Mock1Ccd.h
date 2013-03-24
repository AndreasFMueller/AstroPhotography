/*
 * Mock1Ccd.h -- mock ccd implementation
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Mock1Ccd_h
#define _Mock1Ccd_h

#include <AstroCamera.h>

using namespace astro::image;
using namespace astro::camera;

namespace astro {
namespace camera {
namespace mock1 {

class Mock1Ccd : public Ccd {
	int	cameraid;
	int	ccd;
	ImageRectangle	frame;
public:
	Mock1Ccd(const ImageSize& size, int _cameraid, int _ccd)
		: Ccd(size), cameraid(_cameraid), ccd(_ccd) { }
	virtual ~Mock1Ccd() { }
	virtual void    startExposure(const Exposure& exposure) throw (not_implemented);
	virtual Exposure::State exposureStatus() throw (not_implemented);
	virtual void    cancelExposure() throw (not_implemented);
	virtual ByteImagePtr    byteImage() throw (not_implemented);
	virtual ShortImagePtr   shortImage() throw (not_implemented);
	virtual YUYVImagePtr    yuyvImage() throw (not_implemented);
	virtual RGBImagePtr	rgbImage() throw (not_implemented);
}; 

} // namespace mock1
} // namespace camera
} // namespace astro

#endif /* _Mack1Ccd_h */
