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
	for (size_t offset = 0; offset < image.size().getPixels(); offset++) {
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

#define	do_clamp_monochrome(P)						\
{									\
	Image<P>	*timage = dynamic_cast<Image<P> *>(&*image);	\
	if (NULL != timage) {						\
		do_clamp(*timage, minvalue, maxvalue);			\
		return;							\
	}								\
}

template<typename P>
void	do_clamp(Image<RGB<P> >& image, double minvalue, double maxvalue) {
	for (size_t offset = 0; offset < image.size().getPixels(); offset++) {
		RGB<P>	value = image.pixels[offset];
		// leave NaNs alone
		if ((value.R != value.R) || (value.G != value.G)
			|| (value.B != value.B)) {
			continue;
		}
		if ((value.R < minvalue) || (value.G < minvalue)
			|| (value.B < minvalue)) {
			value = RGB<P>(minvalue, minvalue, minvalue);
		}
		if ((value.R > maxvalue) || (value.G > maxvalue)
			|| (value.B > maxvalue)) {
			value = RGB<P>(maxvalue, maxvalue, maxvalue);
		}
		image.pixels[offset] = value;
	}
}

#define do_clamp_rgb(P)							\
{									\
	Image<RGB<P> >	*timage						\
		= dynamic_cast<Image<RGB<P> > *>(&*image);		\
	if (NULL != timage) {						\
		do_clamp(*timage, minvalue, maxvalue);			\
		return;							\
	}								\
}

void	Clamper::operator()(ImagePtr& image) const {
	do_clamp_monochrome(unsigned char);
	do_clamp_monochrome(unsigned short);
	do_clamp_monochrome(unsigned int);
	do_clamp_monochrome(unsigned long);
	do_clamp_monochrome(float);
	do_clamp_monochrome(double);

	do_clamp_rgb(unsigned char);
	do_clamp_rgb(unsigned short);
	do_clamp_rgb(unsigned int);
	do_clamp_rgb(unsigned long);
	do_clamp_rgb(float);
	do_clamp_rgb(double);
}

} // calibration
} // astro
