/*
 * FastVanCittertOperator.cpp -- implementation 
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConvolve.h>
#include <AstroAdapter.h>
#include <AstroDebug.h>
#include <AstroIO.h>
#include <AstroImageops.h>

namespace astro {
namespace image {

/**
 * \brief Deconvolve an image using the Van Cittert deconvolution algorithm
 *
 * \param image		The image to deconvolve
 */
ImagePtr	FastVanCittertOperator::operator()(ImagePtr image) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "deconvolving %s image in %d iterations",
		image->size().toString().c_str(), iterations());
	// start with the input image and ensure it is double
	adapter::DoubleAdapter	da(image);
	ImagePtr	g = ImagePtr(new Image<double>(da));

	// resize the psf to the size of the image
	FourierImagePtr	psff = fourierpsf(image->size());

	// start iterations
	int	i = iterations();
	while (i--) {
		int	number = iterations() - i;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "iteration %d", number);
		// convolve using the Fourier transform
		FourierImagePtr	gf(new FourierImage(g));
		ImagePtr	convolved = (psff * gf)->inverse();

		// compute the correction
		g = add(image, convolved);
		if (constrained()) {
			ops::positive(g);
		}
		if (prefix().size() > 0) {
			std::string	filename = stringprintf("%s-%02d.fits",
				prefix().c_str(), number);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "writing %s image to %s",
				g->size().toString().c_str(), filename.c_str());
			io::FITSout	out(filename);
			out.setPrecious(false);
			out.write(g);
		}
	}
	// done
	return g;
}

} // namespace astro
} // namespace image
