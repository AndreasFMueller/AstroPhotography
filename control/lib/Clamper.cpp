/**
 * Clamper.cpp -- compute calibration frames
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCalibration.h>

using namespace astro::image;

namespace astro {
namespace calibration {

//////////////////////////////////////////////////////////////////////
// Clamp images to a given range
//////////////////////////////////////////////////////////////////////
Clamper::Clamper(double _minvalue, double _maxvalue)
	: minvalue(_minvalue), maxvalue(_maxvalue) {
}

template<typename P>
void	do_clamp(Image<P>& image, double minvalue, double maxvalue) {
	for (size_t offset = 0; offset < image.size.pixels; offset++) {
		P	value = image.pixels[offset];
		// skip indefined pixels
		if (value != value) {
			continue;
		}
		if (value < minvalue) {
			value = minvalue;
		}
		if (value > maxvalue) {
			value = maxvalue;
		}
		image.pixels[offset] = value;
	}
}

#define	do_clamp_typed(P)						\
{									\
	Image<P>	*timage = dynamic_cast<Image<P> *>(&*image);	\
	if (NULL != timage) {						\
		do_clamp(*timage, minvalue, maxvalue);			\
		return;							\
	}								\
}

void	Clamper::operator()(ImagePtr& image) const {
	do_clamp_typed(unsigned char);
	do_clamp_typed(unsigned short);
	do_clamp_typed(unsigned int);
	do_clamp_typed(unsigned long);
	do_clamp_typed(float);
	do_clamp_typed(double);
}

} // calibration
} // astro
