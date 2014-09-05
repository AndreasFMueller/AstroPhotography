/*
 * PhaseCorrelator.cpp -- find a translation between two images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswli
 */
#include <AstroTransform.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroAdapter.h>
#include <AstroIO.h>
#include <fftw3.h>

using namespace astro::adapter;
using namespace astro::io;

namespace astro {
namespace image {
namespace transform {

static unsigned int	correlation_counter = 1;

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
		int x, int y) const {
	while (x < 0) {
		x += size.width();
	}
	while (x > (int)size.width()) { // cast to make compiler happy
		x -= size.width();
	}
	while (y < 0) {
		y += size.height();
	}
	while (y > (int)size.height()) { // cast to make compiler happy
		y -= size.height();
	}
	return a[size.offset(x, y)];
}

/**
 * \brief Compute 2k+1 x 2k+1 centroid around the center point
 */
Point	PhaseCorrelator::centroid(const double *a, const ImageSize& size,
		const Point& center, unsigned int k) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "computing %d-centroid at %s",
		2 * k + 1, center.toString().c_str());

	// compute the centroid
	double	s = 0;
	double	xs = 0;
	double	ys = 0;
	int	xmin = center.x() - k;
	int	xmax = xmin + 2 * k;
	int	ymin = center.y() - k;
	int	ymax = ymin + 2 * k;
	for (int x = xmin; x <= xmax; x++) {
		for (int y = ymin; y <= ymax; y++) {
			double	v = value(a, size, x, y);
//debug(LOG_DEBUG, DEBUG_LOG, 0, "v(%d, %d) = %f", x, y, v);
			s += v;
			xs += v * x;
			ys += v * y;
		}
	}
	xs /= s;
	ys /= s;
	if (xs > size.width() / 2) {
		xs -= size.width();
	}
	if (ys > size.width() / 2) {
		ys -= size.width();
	}
	return Point(xs, ys);
}

class HanningWindow : public ConstImageAdapter<double> {
	const ConstImageAdapter<double>&	_base;
	double	*hh, *hv;
public:
	HanningWindow(const ConstImageAdapter<double>& base)
		: ConstImageAdapter<double>(base.getSize()), _base(base) {
		// compute the values for the hanning windows
		unsigned int	width = getSize().width();
		hh = new double[width];
		double  h = M_PI / width;
		for (unsigned int x = 0; x < width; x++) {
			hh[x] = sqr(sin(x * h));
		}
		unsigned int	height = getSize().height();
		hv = new double[height];
		h = M_PI / height;
		for (unsigned int y = 0; y < width; y++) {
			hv[y] = sqr(sin(y * h));
		}
	}
	~HanningWindow() {
		delete hh;
		delete hv;
	}
	double	pixel(unsigned int x, unsigned int y) const {
		double	hanning = hh[x] * hv[y];
		return hanning * _base.pixel(x, y);
	}
};

class RectangleWindow : public ConstImageAdapter<double> {
	const ConstImageAdapter<double>&	_base;
	unsigned int	wmin, wmax;
	unsigned int	hmin, hmax;
public:
	RectangleWindow(const ConstImageAdapter<double>& base)
		: ConstImageAdapter<double>(base.getSize()), _base(base) {
		wmin = getSize().width() / 4;
		wmax = 3 * wmin;
		hmin = getSize().height() / 4;
		hmax = 3 * hmin;
	}
	virtual double	pixel(unsigned int x, unsigned int y) const {
		if ((x <= wmin) || (x >= wmax)) {
			return 0;
		}
		if ((y <= hmin) || (y >= hmax)) {
			return 0;
		}
		return _base.pixel(x, y);
	}
};


/**
 * \brief Find displacement between two images using phase correlation.
 *
 * This method applies a Hanning window to the two images, computes the
 * Fourier transforms, takes the product (with the first fourier transform
 * complex conjugated) and computes the reverse transform. Then the maximum
 * is found and a 5x5 centroid around the maximum computed. This gives
 * subpixel accuracy for image translations.
 */
std::pair<Point, double> PhaseCorrelator::operator()(
		const ConstImageAdapter<double>& fromimage,
		const ConstImageAdapter<double>& toimage) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "correlating images %s ~ %s",
		fromimage.getSize().toString().c_str(),
		toimage.getSize().toString().c_str());

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

	// allocate memory for the fourier transforms
	size_t	nc = size.width() * (1 + size.height() / 2);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "pixel count: %u, fourier transform: %u",
		n, nc);
	fftw_complex	*af = (fftw_complex *)fftw_malloc(
					sizeof(fftw_complex) * nc);
	fftw_complex	*bf = (fftw_complex *)fftw_malloc(
					sizeof(fftw_complex) * nc);

	// create a plan for the fourier transform
	fftw_plan	p = fftw_plan_dft_r2c_2d(size.height(), size.width(),
				a, af, 0);
	fftw_plan	q = fftw_plan_dft_r2c_2d(size.height(), size.width(),
				b, bf, 0);

	// we also already need a back transform
	fftw_plan	r = fftw_plan_dft_c2r_2d(size.height(), size.width(),
				af, a, 0);

	// compute the values for the hanning windows
	const ConstImageAdapter<double>	*windowedfrom = NULL;
	const ConstImageAdapter<double>	*windowedto = NULL;
	if (hanning) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using Hanning windows");
		windowedfrom = new HanningWindow(fromimage);
		windowedto = new HanningWindow(toimage);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using Rectangular windows");
		windowedfrom = new RectangleWindow(fromimage);
		windowedto = new IdentityAdapter<double>(toimage);
	}

	// now copy the data into the arrays, applying the hanning window
	// at the same time
	for (unsigned int x = 0; x < size.width(); x++) {
		for (unsigned int y = 0; y < size.height(); y++) {
			a[size.offset(x, y)] = windowedfrom->pixel(x, y);
			b[size.offset(x, y)] = windowedto->pixel(x, y);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "applied window to both images");

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

#if 0
	// write the correlation image for debugging
	//Image<double>	correlation(*windowedto);
	Image<double>	correlation(size);
	memcpy(correlation.pixels, a, n * sizeof(double));
	FITSoutfile<double>	out("correlation.fits");
	out.setPrecious(false);
	out.write(correlation);
#endif

	// find the maximum 
	double	max = 0;
	int maxx = 0;
	int maxy = 0;
	int	w4 = size.width() / 4;
	int	h4 = size.height() / 4;
	for (int x = -w4; x < w4; x++) {
		for (int y = -h4; y < h4; y++) {
			//double	v = a[size.offset(x, y)];
			double	v = value(a, size, x, y);
			if (v > max) {
				max = v;
				maxx = x;
				maxy = y;
			}
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "[%d] maximum at Pixel %d,%d",
		correlation_counter,  maxx, maxy);

	// build the 5x5 centroid to get the best possible Point value
	Point	result = centroid(a, size, Point(maxx, maxy));

#if 1
	// if required, write everything into a single image
try {
	if ((result.x() == result.x()) && (result.y() == result.y())) {
		{
		FITSoutfile<double>	out(stringprintf("corr-from-%u.fits",
						correlation_counter));
		out.setPrecious(false);
		out.write(Image<double>(*windowedfrom));
		}
		{
		FITSoutfile<double>	out(stringprintf("corr-to-%u.fits",
						correlation_counter));
		out.setPrecious(false);
		out.write(Image<double>(*windowedto));
		}
		{
		FITSoutfile<double>	out(stringprintf("corr-%u.fits",
						correlation_counter));
		out.setPrecious(false);
		Image<double>	correlation(size);
		unsigned int	w = size.width();
		unsigned int	h = size.height();
		for (unsigned int x = 0; x < w; x++) {
			for (unsigned int y = 0; y < h; y++) {
				correlation.writablepixel(x, y)
					= value(a, size, x, y);
			}
		}
		correlation.setMetadata(
			FITSKeywords::meta(std::string("XOFFSET"), result.x()));
		correlation.setMetadata(
			FITSKeywords::meta(std::string("YOFFSET"), result.y()));
		out.write(correlation);
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "file %d written",
			correlation_counter);
		correlation_counter++;
	}
} catch (std::exception& x) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exception while writing images: %s",
		x.what());
}
#endif
	
	// clean up the memory allocated
	fftw_destroy_plan(r);
	fftw_destroy_plan(q);
	fftw_destroy_plan(p);
	fftw_free(af);
	fftw_free(bf);
	fftw_cleanup();

	// result
	debug(LOG_DEBUG, DEBUG_LOG, 0, "[%d] translation: %s",
		correlation_counter - 1, result.toString().c_str());
	return std::make_pair(result, max);
}

} // namespace transform
} // namespace image
} // namespace astro
