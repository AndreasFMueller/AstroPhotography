/*
 * PhaseCorrelator.cpp -- find a translation between two images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswli
 */
#include <AstroTransform.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <fftw3.h>

namespace astro {
namespace image {
namespace transform {

static inline double	sqr(double x) {
	return x * x;
}

/**
 * \brief Auxiliary function to retrieve array values
 *
 * When computing the centroid, we often work near the boundary of the domain,
 * this accessor wraps the indices around according to the array size.
 */
double	PhaseCorrelator::value(const double *a, const ImageSize& size,
		unsigned int x, unsigned int y) const {
	while (x > size.getWidth()) {
		x -= size.getWidth();
	}
	while (y > size.getHeight()) {
		y -= size.getHeight();
	}
	return a[size.offset(x, y)];
}

/**
 * \brief Compute 2k+1 x 2k+1 centroid around the center point
 */
Point	PhaseCorrelator::centroid(const double *a, const ImageSize& size,
		const ImagePoint& center, unsigned int k) const {
	unsigned int	cx = center.x;
	if (cx < k) {
		cx += size.getWidth();
	}
	unsigned int	cy = center.y;
	if (cy < k) {
		cy += size.getHeight();
	}
	double	s = 0;
	double	xs = 0;
	double	ys = 0;
	for (unsigned int x = center.x - k; x <= center.x + k; x++) {
		for (unsigned int y = center.y - k; y <= center.y + k; y++) {
			double	v = value(a, size, x, y);
			s += v;
			xs += v * x;
			ys += v * y;
		}
	}
	xs /= s;
	ys /= s;
	if (xs > size.getWidth() / 2) {
		xs -= size.getWidth();
	}
	if (ys > size.getWidth() / 2) {
		ys -= size.getWidth();
	}
	return Point(xs, ys);
}

/**
 * \brief Find displacement between two images using phase correlation.
 *
 * This method applies a Hanning window to the two images, computes the
 * Fourier transforms, takes the product (with the first fourier transform
 * complex conjugated) and computes the reverse transform. Then the maximum
 * is found and a 5x5 centroid around the maximum computed. This gives
 * subpixel accuracy for image translations.
 */
Point	PhaseCorrelator::operator()(const ConstImageAdapter<double>& fromimage,
		const ConstImageAdapter<double>& toimage) {
	// ensure that both images are of the same size
	ImageSize	size = fromimage.getSize();
	if (size != toimage.getSize()) {
		std::string	msg = stringprintf("images differ in size: "
			"%s != %s", size.toString().c_str(),
			toimage.getSize().toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	size_t	n = size.getPixels();
	
	// allocate memory for the images
	double	a[n];
	double	b[n];

	// allocate memeory for the fourier transforms
	size_t	nc = size.getWidth() * (1 + size.getHeight() / 2);
	fftw_complex	*af = (fftw_complex *)fftw_malloc(
					sizeof(fftw_complex) * nc);
	fftw_complex	*bf = (fftw_complex *)fftw_malloc(
					sizeof(fftw_complex) * nc);

	// create a plan for the fourier transform
	fftw_plan	p = fftw_plan_dft_r2c_2d(size.getWidth(), size.getHeight(),
				a, af, 0);
	fftw_plan	q = fftw_plan_dft_r2c_2d(size.getWidth(), size.getHeight(),
				b, bf, 0);

	// we also already need a back transform
	fftw_plan	r = fftw_plan_dft_c2r_2d(size.getWidth(), size.getHeight(),
				af, a, 0);

	// compute the values for the hanning windows
	double	hh[size.getWidth()];
	double	h = M_PI / size.getWidth();
	for (unsigned int x = 0; x < size.getWidth(); x++) {
		hh[x] = sqr(sin(x * h));
	}
	double	hv[size.getHeight()];
	h = M_PI / size.getHeight();
	for (unsigned int y = 0; y < size.getWidth(); y++) {
		hv[y] = sqr(sin(y * h));
	}

	// now copy the data into the arrays, applying the hanning window
	// at the same time
	for (unsigned int x = 0; x < size.getWidth(); x++) {
		for (unsigned int y = 0; y < size.getWidth(); y++) {
			double	hanning = hh[x] * hv[y];
			a[size.offset(x, y)] = hanning * fromimage.pixel(x, y);
			b[size.offset(x, y)] = hanning * toimage.pixel(x, y);
		}
	}

	// now compute the fourier transforms
	fftw_execute(p);
	fftw_execute(q);

	// compute the product of the two fourier transforms
	for (unsigned int i = 0; i < nc; i++) {
		fftw_complex	product;
		product[0] =  af[i][0] * bf[i][0] + af[i][1] * bf[i][1];
		product[1] = -af[i][1] * bf[i][0] + af[i][0] * bf[i][1];
		af[i][0] = product[0];
		af[i][1] = product[1];
	}

	// perform the back transform
	fftw_execute(r);

	// find the maximum 
	double	max = 0;
	unsigned int maxx = 0;
	unsigned int maxy = 0;
	for (unsigned int x = 0; x < size.getWidth(); x++) {
		for (unsigned int y = 0; y < size.getHeight(); y++) {
			double	v = a[size.offset(x, y)];
//std::cout << "v(" << x << "," << y << ") = " << v << std::endl;
			if (v > max) {
				max = v;
				maxx = x;
				maxy = y;
			}
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "maximum at %u,%u", maxx, maxy);

	// build the 5x5 centroid to get the best possible Point value
	Point	result = centroid(a, size, ImagePoint(maxx, maxy));
	
	// clean up the memory allocated
	fftw_destroy_plan(r);
	fftw_destroy_plan(q);
	fftw_destroy_plan(p);
	fftw_free(af);
	fftw_free(bf);
	fftw_cleanup();

	// result
	return result;
}

} // namespace transform
} // namespace image
} // namespace astro
