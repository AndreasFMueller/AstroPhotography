/*
 * FourierImage.cpp -- implementation of the Fourier image class
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConvolve.h>
#include <fftw3.h>
#include <math.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroAdapter.h>

namespace astro {
namespace image {

/**
 * \brief Compute size of the complex fourier transform image
 *
 * We are using the real data DFTs from the fftw3 library, which uses a layout
 * different from what you would expect from our image types. When going 
 * through a pixel array In our image types, the quickly increasing
 * coordinate is the horizontal coordinate, which we usually call the
 * x coordinate, and which is also the first coordinate. In FFTW3, the slowly
 * increasing coordinate when going through the FFT array is the second
 * coordinate. So if an image has width w and height h, then we have
 * treat it as a data array with n0 = h and n1 = w. The corresponding
 * fourier transform array for the real data transforms then has dimensions
 * n0 and (n1/2 + 1). But since again the second coordinate is the one that
 * increases quickly, we have to create an image of width (n1/2 + 1) and
 * height n0.
 *
 * All this is unimportant as long as we don't look at the fourier transform
 * as an image in its own right. Only then does it become important how we
 * interpret the coordinates.
 *
 * \param size	size of the image to be fourier transformed
 */
ImageSize	FourierImage::fsize(const ImageSize& size) {
	int	w = size.width();
	int	h = size.height();
	int	n0 = h;
	int	n1 = w;
	ImageSize	result(2 * (1 + n1 / 2), n0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "fourier image size %s -> %s",
		size.toString().c_str(), result.toString().c_str());
	return result;
}

/**
 * \brief Perform the fourier transform
 *
 * This method uses the real-data fourier transform from the FFTW3 library
 * to compute the fourier transform of the real pixel data. To match the
 * different conventions how image data is stored, the ny or n1 dimension
 * of the fftw data array needs to be the width of the image.
 */
void	FourierImage::fourier(const Image<double>& image) {
	// make sure image has the right dimensions (the ones we stored in
	// _orig)
	if (_orig != image.size()) {
		throw std::range_error("wrong dims for fourier transform");
	}

	// get the dimensions (note the differing data storage conventions
	// used in fftw3 and in our image class)
	int	n0 = image.size().height();
	int	n1 = image.size().width();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "fourier transform dimensions: %d x %d",
		n0, n1);

	// compute the fourier transform
	fftw_plan	p = fftw_plan_dft_r2c_2d(n0, n1, image.pixels,
				(fftw_complex *)pixels, FFTW_ESTIMATE);
	fftw_execute(p);
	fftw_destroy_plan(p);
	fftw_cleanup();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "fourier transform completed");
}

/**
 * \brief Construct a FourierTransform object from the size
 */
FourierImage::FourierImage(const ImageSize& s)
	: Image<double>(FourierImage::fsize(s)), _orig(s) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "construct fourier image of size %s",
		size().toString().c_str());
	fill(0);
}

/**
 * \brief Construct a FourierTransform object from a double image
 */
FourierImage::FourierImage(const Image<double>& image)
	: Image<double>(FourierImage::fsize(image.size())),
	  _orig(image.size()) {
	fourier(image);
}

/**
 * \brief Construct a FourierTransform from an image adapter
 */
FourierImage::FourierImage(const ConstImageAdapter<double>& image)
	: Image<double>(FourierImage::fsize(image.getSize())),
	  _orig(image.getSize()) {
	Image<double>	i(image);
	fourier(i);
}

/**
 * \brief Construct a FourierTransform object from any type of image
 */
FourierImage::FourierImage(const ImagePtr image)
	: Image<double>(FourierImage::fsize(image->size())),
	  _orig(image->size()) {
	// first handle the case where the image in fact already is a double
	// image
	const Image<double>	*img = dynamic_cast<Image<double>*>(&*image);
	if (NULL != img) {
		fourier(*img);
		return;
	}

	// All other cases need an adapter to convert the image into a
	// double image first
	debug(LOG_DEBUG, DEBUG_LOG, 0, "build a double adapter");
	adapter::DoubleAdapter	a(image);
	Image<double>	in(a);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "double image of size %s constructed",
		in.size().toString().c_str());
	fourier(in);
}

/**
 * \brief Compute the inverse transform
 *
 * Note that in order to get inverse, we also have to divide by the volume
 * of the domain, which explains why we do all this only for float valued
 * pixels.
 */
ImagePtr	FourierImage::inverse(bool absolute) const {
	Image<double>	*image = new Image<double>(_orig);
	int	n0 = _orig.height();
	int	n1 = _orig.width();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "inverse transform, (%d,%d)", n0, n1);
	// compute the fourier transform
	fftw_plan	p = fftw_plan_dft_c2r_2d(n0, n1, (fftw_complex *)pixels,
				image->pixels, FFTW_ESTIMATE);
	fftw_execute(p);
	fftw_destroy_plan(p);
	fftw_cleanup();

	// normalize to the dimensions of the domain
	double	value = 1. / (n0 * n1);
	int	w = _orig.width();
	int	h = _orig.height();
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			double	v = image->pixel(x, y) * value;
			if (absolute) {
				if (v < 0) {
					v = -v;
				}
			}
			image->pixel(x, y) = v;
		}
	}
	return ImagePtr(image);
}

/**
 * \brief Compute the absolute value of the complex fourier transform
 */
ImagePtr	FourierImage::abs() const {
	// dimensions of the resulting image
	int	w = size().width() / 2;
	int	h = size().height();

	int	n0 = size().width() / 2;
	int	n1 = size().height();
	
	Image<double>	*image =  new Image<double>(n0, n1);
	fftw_complex	*a = (fftw_complex *)pixels;
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			int	offset = x + w * y;
			image->pixel(x, y) = hypot(a[offset][0], a[offset][1]);
		}
	}
	return ImagePtr(image);
}

/**
 * \brief Compute the phase of the complex fourier transform
 */
ImagePtr	FourierImage::phase() const {
	// dimensions of the resulting image
	int	w = size().width() / 2;
	int	h = size().height();

	int	n0 = size().width() / 2;
	int	n1 = size().height();
	
	Image<double>	*image =  new Image<double>(n0, n1);
	fftw_complex	*a = (fftw_complex *)pixels;
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			int	offset = x + w * y;
			image->pixel(x, y) = atan2(a[offset][1], a[offset][0]);
		}
	}
	return ImagePtr(image);
}

/**
 * \brief Compute the absolute value of the complex fourier transform
 */
ImagePtr	FourierImage::color() const {
	// dimensions of the resulting image
	int	w = size().width() / 2;
	int	h = size().height();

	int	n0 = size().width() / 2;
	int	n1 = size().height();

	// get access to the image
	Image<RGB<double> >	*image =  new Image<RGB<double> >(n0, n1);
	fftw_complex	*a = (fftw_complex *)pixels;

	// first find the largest value
	double	rmax = 0;
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			int	offset = x + w * y;
			double	r = hypot(a[offset][0], a[offset][1]);
			if (r > rmax) {
				rmax = r;
			}
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "maximum r-value: %f", rmax);
	
	// convert radius and argument to a color pixel
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			int	offset = x + w * y;
			double	r = hypot(a[offset][0], a[offset][1]) / rmax;
			double	phi = atan2(a[offset][1], a[offset][0]);
			HSL<double>	h(phi, 1, r);
			image->pixel(x, y) = RGB<double>(h);
		}
	}
	return ImagePtr(image);
}

/**
 * \brief Compute the product of two fourier transforms
 *
 * Upon reverse transform, this becomes the convolution product of the
 * original functions
 */
FourierImagePtr	operator*(const FourierImage& a, const FourierImage& b) {
	if (a.size() != b.size()) {
		std::string	msg = stringprintf(
			"image size mismatch: %s != %s",
			a.orig().toString().c_str(),
			b.orig().toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// construct the result image
	FourierImage	*result = new FourierImage(a.orig());

	// compute the product
	fftw_complex	*af = (fftw_complex *)a.pixels;
	fftw_complex	*bf = (fftw_complex *)b.pixels;
	fftw_complex	*cf = (fftw_complex *)result->pixels;
	size_t	nc = result->size().getPixels() / 2;
	for (unsigned int i = 0; i < nc; i++) {
		cf[i][0] = af[i][0] * bf[i][0] - af[i][1] * bf[i][1];
                cf[i][1] = af[i][1] * bf[i][0] + af[i][0] * bf[i][1];
	}

	// return the new image
	return FourierImagePtr(result);
}

/**
 * \brief Compute the quotient of two fourier transforms
 *
 * Upon reverse transform, this becomes the deconvolution of the
 * original functions
 */
FourierImagePtr	operator/(const FourierImage& a, const FourierImage& b) {
	if (a.size() != b.size()) {
		std::string	msg = stringprintf(
			"image size mismatch: %s != %s",
			a.orig().toString().c_str(),
			b.orig().toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// construct the result image
	FourierImage	*result = new FourierImage(a.orig());

	// compute the product
	fftw_complex	*af = (fftw_complex *)a.pixels;
	fftw_complex	*bf = (fftw_complex *)b.pixels;
	fftw_complex	*cf = (fftw_complex *)result->pixels;
	size_t	nc = result->size().getPixels() / 2;
	for (unsigned int i = 0; i < nc; i++) {
		double	d = bf[i][0] * bf[i][0] + bf[i][1] * bf[i][1];
		cf[i][0] = (af[i][0] * bf[i][0] + af[i][1] * bf[i][1]) / d;
                cf[i][1] = (af[i][1] * bf[i][0] - af[i][0] * bf[i][1]) / d;
	}

	// return the new image
	return FourierImagePtr(result);
}

/**
 * \brief Compute the product of two fourier transforms (ptr version)
 */
FourierImagePtr	operator*(const FourierImagePtr a, const FourierImagePtr b) {
	return (*a) * (*b);
}

/**
 * \brief Compute the quotient of two fourier transforms (ptr version)
 */
FourierImagePtr	operator/(const FourierImagePtr a, const FourierImagePtr b) {
	return (*a) / (*b);
}

/**
 * \brief Compute the pseudoinverse of the deconvolution the the PSF b
 *
 * \param a		the fourier transform of the image
 * \param b		the fourier transform of the psf
 * \param epsilon	the limit of small coefficients to ignore
 */
FourierImagePtr	pseudo(const FourierImage& a, const FourierImage& b,
			double epsilon) {
	if (a.size() != b.size()) {
		std::string	msg = stringprintf(
			"image size mismatch: %s != %s",
			a.orig().toString().c_str(),
			b.orig().toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// construct the result image
	FourierImage	*result = new FourierImage(a.orig());

	// compute the product
	fftw_complex	*af = (fftw_complex *)a.pixels;
	fftw_complex	*bf = (fftw_complex *)b.pixels;
	fftw_complex	*cf = (fftw_complex *)result->pixels;
	size_t	nc = result->size().getPixels() / 2;
	for (unsigned int i = 0; i < nc; i++) {
		double	d = bf[i][0] * bf[i][0] + bf[i][1] * bf[i][1];
		if (d < epsilon) {
			cf[i][0] = 0;
			cf[i][1] = 0;
			continue;
		}
		cf[i][0] = (af[i][0] * bf[i][0] + af[i][1] * bf[i][1]) / d;
                cf[i][1] = (af[i][1] * bf[i][0] - af[i][0] * bf[i][1]) / d;
	}

	// return the new image
	return FourierImagePtr(result);
}

/**
 * \brief Compute the pseudoinverse of the deconvolution the the PSF b
 *
 * \param a		the fourier transform of the image
 * \param b		the fourier transform of the psf
 * \param K		the limit of small coefficients to ignore
 */
FourierImagePtr	wiener(const FourierImage& a, const FourierImage& b,
			double K) {
	if (a.size() != b.size()) {
		std::string	msg = stringprintf(
			"image size mismatch: %s != %s",
			a.orig().toString().c_str(),
			b.orig().toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// construct the result image
	FourierImage	*result = new FourierImage(a.orig());

	// compute the product
	fftw_complex	*af = (fftw_complex *)a.pixels;
	fftw_complex	*bf = (fftw_complex *)b.pixels;
	fftw_complex	*cf = (fftw_complex *)result->pixels;
	size_t	nc = result->size().getPixels() / 2;
	for (unsigned int i = 0; i < nc; i++) {
		double	d = bf[i][0] * bf[i][0] + bf[i][1] * bf[i][1] + K;
		cf[i][0] = (af[i][0] * bf[i][0] + af[i][1] * bf[i][1]) / d;
                cf[i][1] = (af[i][1] * bf[i][0] - af[i][0] * bf[i][1]) / d;
	}

	// return the new image
	return FourierImagePtr(result);
}

FourierImagePtr	wiener(const FourierImagePtr a, const FourierImagePtr b,
			double epsilon) {
	return wiener(*a, *b, epsilon);
}

} // namespace image
} // namespace astro
