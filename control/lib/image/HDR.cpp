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
	}								\
}

/**
 * \brief Operator to rescale an image
 */
ImagePtr	HDR::operator()(ImagePtr image) const {
	// convert the mask image to double, as we have to fourier transform it
	FourierImage	*fmask = NULL;
	typeconvert(unsigned char, mask());
	typeconvert(unsigned short, mask());
	typeconvert(unsigned int, mask());
	typeconvert(unsigned long, mask());
	typeconvert(float, mask());
	typeconvert(double, mask());
	if (NULL == fmask) {
		std::runtime_error("cannot work with this mask type");
	}
	FourierImagePtr	fmaskptr(fmask);

	// get a gaussian blurring function
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create %s gauss with radius %f",
		mask()->size().toString().c_str(), radius());
	TiledGaussImage	tg(mask()->size(), radius(), 1);
	Image<double>	*tgimage = new Image<double>(tg);
	ImagePtr	tgimageptr(tgimage);
	FourierImage	blurr(*tgimage);

	// convolve
	FourierImagePtr	blurred = (*fmask) * blurr;
	ImagePtr	blurredmaskptr = blurred->inverse();
	Image<double>	*blurredmask
		= dynamic_cast<Image<double>*>(&*blurredmaskptr);

#if 1
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
