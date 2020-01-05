/*
 * DeconvolutionOperator.cpp -- Base class implementation for deconvolution
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroConvolve.h>
#include <AstroIO.h>
#include <AstroAdapter.h>

namespace astro {
namespace image {

/**
 * \brief Macro to convert the PSF image into a 
 */
#define psf_typed(psf, Pixel)						\
{									\
	Image<Pixel >	*img = dynamic_cast<Image<Pixel > *>(&*psf);	\
	if (NULL != img) {						\
		adapter::LuminanceAdapter<Pixel, double> la(*img);	\
		_psf = la;						\
		copied = true;						\
	}								\
}

/**
 * \brief Build a PSF image with a given size
 *
 * \param size		size of the image to build
 */
FourierImagePtr	DeconvolutionOperator::fourierpsf(const ImageSize& size) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating the fourier image of psf");
	Image<double>	*fpsf = new Image<double>(size);
	fpsf->fill(0.);
	ImagePtr	fpsfptr(fpsf); // resource managment

	// find the center of the psf
	ImagePoint	center = _psf.center();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "psf center: %s",
		center.toString().c_str());

	// copy psf into the image
	int	w = _psf.size().width();
	int	h = _psf.size().height();
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			int	dx = x - center.x();
			int	dy = y - center.y();
			int	xx = (dx < 0) ? (size.width() + dx) : dx;
			int	yy = (dy < 0) ? (size.height() + dy) : dy;
			fpsf->pixel(xx, yy) = _psf.pixel(x, y);
		}
	}

	// write the large psf
	if (debuglevel > 0) {
		io::FITSout	out("largepsf.fits");
		out.setPrecious(false);
		out.write(fpsfptr);
	}

	// fourier transform the psf image
	FourierImagePtr	result = FourierImagePtr(new FourierImage(fpsfptr));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "psf fourier transformed");
	return result;
}

/**
 * \brief Construct a FourierDeconvolutionOperator
 *
 * \param psf	the image to use as a point spread function
 */
DeconvolutionOperator::DeconvolutionOperator(ImagePtr psf) : _psf(psf->size()) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "construct with psf of size %s",
		psf->size().toString().c_str());
	bool	copied = false;
	// copy the data from the argument psf, this depends on the type
	psf_typed(psf, unsigned char)
	psf_typed(psf, unsigned short)
	psf_typed(psf, unsigned int)
	psf_typed(psf, unsigned long)
	psf_typed(psf, float)
	psf_typed(psf, double)
	psf_typed(psf, RGB<unsigned char>)
	psf_typed(psf, RGB<unsigned short>)
	psf_typed(psf, RGB<unsigned int>)
	psf_typed(psf, RGB<unsigned long>)
	psf_typed(psf, RGB<float>)
	psf_typed(psf, RGB<double>)
	psf_typed(psf, YUYV<unsigned char>)
	psf_typed(psf, YUYV<unsigned short>)
	psf_typed(psf, YUYV<unsigned int>)
	psf_typed(psf, YUYV<unsigned long>)
	psf_typed(psf, YUYV<float>)
	psf_typed(psf, YUYV<double>)
	if (!copied) {
		throw std::runtime_error("no acceptable pixel type");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a psf of size %s",
		psf->size().toString().c_str());
}

/**
 * \brief Construct a FourierDeconvolutionOperator
 *
 * \param psf	the image to use as a point spread function
 */
DeconvolutionOperator::DeconvolutionOperator(
	const ConstImageAdapter<double>& psf) : _psf(psf) {
}


} // namespace image
} // namespace astro


