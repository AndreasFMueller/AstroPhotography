/*
 * PhaseCorrelator.cpp -- find a translation between two images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswli
 */
#include <AstroTransform.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroAdapter.h>
#include <AstroFilter.h>
#include <AstroIO.h>
#include <fftw3.h>
#include <includes.h>

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
 * \brief Adapter class to extract the image in a hanning window 
 */
class HanningWindow : public ConstImageAdapter<double> {
	const ConstImageAdapter<double>&	_base;
	double	*hh, *hv;
	// prevent copying
	HanningWindow(const HanningWindow& other);
	HanningWindow&	operator=(const HanningWindow& other);
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
		for (unsigned int y = 0; y < height; y++) {
			hv[y] = sqr(sin(y * h));
		}
	}
	~HanningWindow() {
		delete hh;
		delete hv;
	}
	double	pixel(int x, int y) const {
		double	hanning = hh[x] * hv[y];
		return hanning * _base.pixel(x, y);
	}
};

/**
 * \brief Adapter class to extract a rectangle
 */
class RectangleWindow : public ConstImageAdapter<double> {
	const ConstImageAdapter<double>&	_base;
	int	wmin, wmax;
	int	hmin, hmax;
public:
	RectangleWindow(const ConstImageAdapter<double>& base)
		: ConstImageAdapter<double>(base.getSize()), _base(base) {
		wmin = getSize().width() / 4;
		wmax = 3 * wmin;
		hmin = getSize().height() / 4;
		hmax = 3 * hmin;
	}
	virtual double	pixel(int x, int y) const {
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
 * \brief auxiliary function to write phase correlation images 
 *
 * This method is only used when debugging, in fact it returns very quickly
 * if debugging is not on. It it runs, it creates a unique filename
 * and writes the contents of the image to it.
 */
void	PhaseCorrelator::write(const Image<double>& image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "write request for %s image",
		image.size().toString().c_str());
	if (debuglevel < LOG_DEBUG) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "not debugging");
		return;
	}
	if (_imagedir.size() == 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image directory not set");
		return;
	}
	struct stat	sb;
	if (stat(_imagedir.c_str(), &sb) < 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image directory not found");
		return;
	}
	std::string	filename = stringprintf("%s/%s-%05d.fits",
		_imagedir.c_str(), _prefix.c_str(),
		correlation_counter);
	try {
		FITSoutfile<double>	out(filename);
		out.setPrecious(false);
		out.write(image);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"exception %s while writing %s: %s",
			demangle(typeid(x).name()).c_str(), filename.c_str(),
			 x.what());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "file %s written, counter = %d",
		filename.c_str(), correlation_counter);
	correlation_counter++;
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image has %lu pixels", n);
	
	// allocate memory for the images
	double	*a = new double[n];
	double	*b = new double[n];

	// allocate memory for the fourier transforms
	size_t	nc = size.width() * (1 + size.height() / 2);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "pixel count: %lu, "
		"fourier transform pixel count: %lu", n, nc);
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
	if (_hanning) {
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
	for (int x = 0; x < size.width(); x++) {
		for (int y = 0; y < size.height(); y++) {
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

	// perform the reverse Fourier transform
	fftw_execute(r);

	// construct an adapter tothe array containing the fourier transform
	ArrayAdapter<double>	aa(a, size);
	ImagePoint	center(size.width() / 2, size.height() / 2);
	TilingAdapter<double>	ta(aa, center);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "center of %s image: %s",
		size.toString().c_str(), center.toString().c_str());

	// search for the maximum in a rectangle
	ImagePoint	lowerleft(size.width() / 4, size.height() / 4);
	ImageRectangle	frame(lowerleft,
				ImageSize(size.width() / 2, size.height() / 2));
	WindowAdapter<double>	wa(ta, frame);
	filter::Max<double, double>	maxfilter;
	double	max = maxfilter(wa);
	ImagePoint	maxcandidate = maxfilter.getPoint() + lowerleft;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "maximum candidate: %s",
		(maxcandidate - center).toString().c_str());

	// construct a peak finder 
	filter::PeakFinder	pf(maxcandidate, 20);
	Point	result = pf(ta) - center;

	// if required, write everything into a single image
	if ((result.x() == result.x()) && (result.y() == result.y())) {
		Image<double>	composite(3 * size.width(), size.height());

		// copy the from image into a subimage at left
		SubimageAdapter<double>	fromsubimage(composite,
			ImageRectangle(ImagePoint(0,0), size));
		NormalizationAdapter<double>	fromnorm(*windowedfrom);
		copy(fromsubimage, fromnorm);

		// copy the from image into a subimage at center
		SubimageAdapter<double>	tosubimage(composite,
			ImageRectangle(ImagePoint(size.width(),0), size));
		NormalizationAdapter<double>	tonorm(*windowedto);
		copy(tosubimage, tonorm);

		// copy the correlation image into a subimage at right
		SubimageAdapter<double>	corrsubimage(composite,
			ImageRectangle(ImagePoint(2 * size.width(),0), size));
		NormalizationAdapter<double>	corrnorm(ta);
		copy(corrsubimage, corrnorm);
		
		// add metadata about the offset
		composite.setMetadata(
			FITSKeywords::meta(std::string("XOFFSET"), result.x()));
		composite.setMetadata(
			FITSKeywords::meta(std::string("YOFFSET"), result.y()));
		write(composite);
	}
	
	// clean up the memory allocated
	fftw_destroy_plan(r);
	fftw_destroy_plan(q);
	fftw_destroy_plan(p);
	fftw_free(af);
	fftw_free(bf);
	fftw_cleanup();

	// we should now remove the window adapters
	if (windowedfrom) { delete windowedfrom; windowedfrom = NULL; }
	if (windowedto)   { delete windowedto;   windowedto = NULL;   }

	// at this point we no longer need the a and b arrays, so we free
	// them in order not to forget this later
	delete[] a;
	delete[] b;

	// result
	debug(LOG_DEBUG, DEBUG_LOG, 0, "[%d] translation: %s",
		correlation_counter - 1, result.toString().c_str());
	return std::make_pair(result, max);
}


#if 0
//////////////////////////////////////////////////////////////////////
// derivative phase correlator implementation
//////////////////////////////////////////////////////////////////////

std::pair<Point, double>	DerivativePhaseCorrelator::operator()(
	const ConstImageAdapter<double>& fromimage,
	const ConstImageAdapter<double>& toimage) {
	DerivativeNormAdapter<double>	i1(fromimage);
	DerivativeNormAdapter<double>	i2(toimage);
	return PhaseCorrelator::operator()(i1, i2);
}
#endif

} // namespace transform
} // namespace image
} // namespace astro
