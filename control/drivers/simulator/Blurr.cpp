/*
 * Blurr.cpp -- Compute the focus blurr
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Blurr.h>
#include <fftw3.h>
#include <math.h>
#include <AstroDebug.h>

namespace astro {
namespace image {

static double	sqr(double x) { return x * x; }

/**
 * \brief Compute the blurr
 */
void	Blurr::operator()(const Image<double>& image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "computing the convolution for blurr");
	// how large is the pixel array that we should use for the
	// computation
	size_t	n = image.size().getPixels();
	int	n0 = image.size().height();
	int	n1 = image.size().width();
	size_t	nc = n0 * (1 + (n1 / 2));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "nc = %d", nc);

	// compute the blurring function
	Image<double>	blurr(ImageSize(n1, n0));

	// allocate memory for the transform
	fftw_complex	*af = (fftw_complex *)fftw_malloc(
					sizeof(fftw_complex) * nc);
	fftw_complex	*bf = (fftw_complex *)fftw_malloc(
					sizeof(fftw_complex) * nc);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "transform memory allocated");

	// plan the fourier transform
	fftw_plan	p = fftw_plan_dft_r2c_2d(n0, n1, 
				image.pixels, af, FFTW_ESTIMATE);
	fftw_plan	q = fftw_plan_dft_r2c_2d(n0, n1, 
				blurr.pixels, bf, FFTW_ESTIMATE);
	fftw_plan	r = fftw_plan_dft_c2r_2d(n0, n1,
				af, image.pixels, FFTW_ESTIMATE);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "transforms planned");

	// compute the values
	double	value = 1 / (M_PI * (sqr(_radius) - sqr(_innerradius)) * n0 * n1);
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"radius = %.1f, innerradius = %.1f, value = %f",
		_radius, _innerradius, value);
	for (unsigned int x = 0; x < n1; x++) {
		for (unsigned int y = 0; y < n0; y++) {
			int	xx = (x > (n0 / 2)) ? (n1 - x) : x;
			int	yy = (y > (n1 / 2)) ? (n0 - y) : y;
			double	r = hypot(xx, yy);
			if ((_innerradius <= r) && (r <= _radius)) {
				blurr.pixel(x, y) = value;
			} else {
				blurr.pixel(x, y) = 0;
			}
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "blurr kernel computed");

	// compute the fourier transforms
	fftw_execute(p);
	fftw_execute(q);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "transform computed");

	// compute the product
	for (unsigned int i = 0; i < nc; i++) {
		fftw_complex	product;
		product[0] = af[i][0] * bf[i][0] - af[i][1] * bf[i][1];
		product[1] = af[i][1] * bf[i][0] + af[i][0] * bf[i][1];
		af[i][0] = product[0];
		af[i][1] = product[1];
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "product computed");

	// compute the inverse fourier transform
	fftw_execute(r);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "inverse transform computed");

	// clean up the memory allocated
	fftw_destroy_plan(r);
	fftw_destroy_plan(q);
	fftw_destroy_plan(p);
	fftw_free(af);
	fftw_free(bf);
	fftw_cleanup();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "blurr computation complete");
}

} // namespace image
} // namespace astro
