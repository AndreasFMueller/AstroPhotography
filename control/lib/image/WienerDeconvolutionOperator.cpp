/*
 * WienerDeconvolutionOperator.cpp
 *
 * (c) 2020 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConvolve.h>
#include <AstroIO.h>

namespace astro {
namespace image {

/**
 * \brief Deconvolve an image
 *
 * \param image		the image to deconvolve
 */
ImagePtr	WienerDeconvolutionOperator::operator()(ImagePtr image) const {
	// bring the image to the size of the image
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get Fourier transform of psf");
	FourierImagePtr	psfptr = fourierpsf(image->size());

	// fourier transform the image
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get Fourier transform of image");
	FourierImage	*fimage = new FourierImage(image);
	FourierImagePtr	fimageptr = FourierImagePtr(fimage);

	// perform the deconvolving
	debug(LOG_DEBUG, DEBUG_LOG, 0, "deconvolve");
	return wiener(fimageptr, psfptr, K())->inverse(true);
}

} // namespace image
} // namespace astro
