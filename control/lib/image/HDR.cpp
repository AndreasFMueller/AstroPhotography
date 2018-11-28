/*
 * HDR.cpp -- postprocessing class for HDR
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroPostprocessing.h>
#include <AstroAdapter.h>
#include <AstroFilter.h>
#include <AstroAdapter.h>
#include <AstroConvolve.h>
#include <AstroIO.h>

namespace astro {
namespace image {
namespace post {

using namespace astro::image;
using namespace astro::adapter;
using namespace astro::image::filter;

/**
 * \brief Construct a Rescale object
 */
HDR::HDR() : _radius(1), _degree(-1) {
}

#define typeconvert(Pixel, maskptr)					\
{									\
	Image<Pixel>	*maskimage					\
		= dynamic_cast<Image<Pixel>*>(&*maskptr);		\
	if (NULL != maskimage) {					\
		adapter::TypeConversionAdapter<Pixel>	tca(*maskimage);\
		fmask = new FourierImage(tca);				\
		monomask = maskptr;					\
	}								\
}

#define typeconvert_rgb(Pixel, maskptr)					\
{									\
	Image<RGB<Pixel> >	*maskimage				\
		= dynamic_cast<Image<RGB<Pixel> >*>(&*maskptr);		\
	if (NULL != maskimage) {					\
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using luminance");	\
		adapter::LuminanceAdapter<RGB<Pixel>, double>		\
			la(*maskimage);					\
		Image<double>	*maskimage = new Image<double>(la);	\
		monomask = ImagePtr(maskimage);				\
		fmask = new FourierImage(*maskimage);			\
	}								\
}

/**
 * \brief Operator to rescale an image
 */
ImagePtr	HDR::operator()(ImagePtr image) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "apply HDR algorithm, radius=%f",
		radius());
	ImagePtr	monomask;
	// convert the mask image to double, as we have to fourier transform it
	FourierImage	*fmask = NULL;
	typeconvert(unsigned char, mask())
	typeconvert(unsigned short, mask())
	typeconvert(unsigned int, mask())
	typeconvert(unsigned long, mask())
	typeconvert(float, mask())
	typeconvert(double, mask())
	typeconvert_rgb(unsigned char, mask())
	typeconvert_rgb(unsigned short, mask())
	typeconvert_rgb(unsigned int, mask())
	typeconvert_rgb(unsigned long, mask())
	typeconvert_rgb(float, mask())
	typeconvert_rgb(double, mask())
	if (NULL == fmask) {
		std::string	msg("cannot work with this mask type");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	FourierImagePtr	fmaskptr(fmask); // resource management
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Fourier mask prepared");

	// get a gaussian blurring function
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create %s gauss with radius %f",
		mask()->size().toString().c_str(), radius());
	TiledGaussImage	tg(mask()->size(), radius(), 1);
	Image<double>	*tgimage = new Image<double>(tg);
	ImagePtr	tgimageptr(tgimage); // resource management
	FourierImage	blurr(*tgimage);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "blurr function fourier transform");

	// convolve
	FourierImagePtr	blurred = (*fmask) * blurr;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "blurred computed");
	ImagePtr	blurredmaskptr = blurred->inverse();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "inverse computed");
	Image<double>	*blurredmask
		= dynamic_cast<Image<double>*>(&*blurredmaskptr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "blurredmask computed");

#if 0
	io::FITSout	maskout("maskout.fits");
	maskout.setPrecious(false);
	maskout.write(monomask);
	io::FITSout	blurrout("blurrout.fits");
	blurrout.setPrecious(false);
	blurrout.write(tgimageptr);
	io::FITSout	blurredout("blurredout.fits");
	blurredout.setPrecious(false);
	blurredout.write(blurredmaskptr);
#endif

	// process the image file
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %s-image of type %s", 
		image->size().toString().c_str(),
		demangle(image->pixel_type().name()).c_str());

	// hdr masking of image
	debug(LOG_DEBUG, DEBUG_LOG, 0, "deemphasize by %f", degree());
	ImagePtr	outimage = adapter::deemphasize(image, *blurredmask,
		degree());
	return outimage;
}

} // namespace post
} // namespace image
} // namespace astro
