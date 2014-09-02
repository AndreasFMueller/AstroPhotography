/*
 * CalibrationProcessorStep.cpp -- implementation of creators for calibration
 *                                 images
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <AstroDebug.h>
#include <AstroFilterfunc.h>

using namespace astro::adapter;

namespace astro {
namespace process {

/**
 * \brief Create a new calibration processor
 */
CalibrationProcessorStep::CalibrationProcessorStep(CalibrationImageStep::caltype t)
	: CalibrationImageStep(t) {
	rawimages = NULL;
	nrawimages = 0;
	_spacing = 1;
	_step = 10;
	medians = NULL;
	means = NULL;
	stddevs = NULL;
}

/**
 * \brief Destroy the calibration processor
 */
CalibrationProcessorStep::~CalibrationProcessorStep() {
	if (NULL != rawimages) {
		delete rawimages;
		rawimages = NULL;
	}
}

/**
 * \brief Get tile x coordinate from image coordinates
 */
int	CalibrationProcessorStep::xt(int x) const {
	return _spacing * (x / (2 * _step));
}

/**
 * \brief Get tile y coordinate from image coordinates
 */
int	CalibrationProcessorStep::yt(int y) const {
	return _spacing * (y / (2 * _step));
}

/**
 * \brief Center x coordinate from image coordinate
 */
int	CalibrationProcessorStep::xc(int x) const {
	return xi(x / (2 * _step));
}

/**
 * \brief Center y coordinate from image coordinate
 */
int	CalibrationProcessorStep::yc(int y) const {
	return yi(y / (2 * _step));
}

/**
 * \brief Image x coordinate from tile coordinate
 */
int	CalibrationProcessorStep::xi(int x) const {
	return _step * (1 + 2 * x);
}

/**
 * \brief Image y coordinate from tile coordinate
 */
int	CalibrationProcessorStep::yi(int y) const {
	return _step * (1 + 2 * y);
}

/**
 * \brief Get size for the tile images for aggregates
 */
ImageSize	CalibrationProcessorStep::tileimagesize(const ImageSize& size) const {
	int	s = _spacing * _step * 2;
	return ImageSize(_spacing * (1 + size.width() / s),
			_spacing * (1 + size.height() / s));
}

/**
 * \brief Find all RawImage precursors
 *
 * get pointers to all the precursors of type ImageStep, others don't have
 * image data output, so they cannot be used to build calibration images
 */
size_t	CalibrationProcessorStep::getPrecursors() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testing %d precursors",
		precursors().size());
	typedef ImageStep *ImageStepP;
	rawimages = new ImageStepP[precursors().size()];
	nrawimages = 0;

	// the lambda below cannot capture members, so we have to capture
	// local variables
	ImageStep	**ri = rawimages;
	for (unsigned int i = 0; i < precursors().size(); i++) {
		rawimages[i] = NULL;
	}
	int	n = 0;
	int	*np = &n;
	std::for_each(precursors().begin(), precursors().end(),
		[ri,np](ProcessingStep *step) mutable {
			ImageStep	*f = dynamic_cast<ImageStep *>(step);
			if (NULL != f) {
				ri[(*np)++] = f;
			}
		}
	);
	nrawimages = n;

	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d raw images", nrawimages);
	return nrawimages;
}

/**
 * \brief compute the mean of an array of doubles
 */
static double	mean(const double *x, int n) {
	double	sum = 0;
	for (int i = 0; i < n; i++) {
		sum += x[i];
	}
	return sum / n;
}

/**
 * \brief Common work for both ClabrationProcessors
 *
 * This step essentially takes care of getting all the precursor images
 */
ProcessingStep::state	CalibrationProcessorStep::common_work() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "common: getting precursor images");
	if (NULL != rawimages) {
		delete rawimages;
		rawimages = NULL;
		nrawimages = 0;
	}
	getPrecursors();
	if (nrawimages == 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no raw images found");
		return ProcessingStep::idle;
	}

	// ensure all images have the same size
	ImageSize	size = rawimages[0]->out().getSize();
	for (size_t i = 1; i < nrawimages; i++) {
		ImageSize	newsize = rawimages[i]->out().getSize();
		if (newsize != size) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"image %d differs in size: %s != %s",
				newsize.toString().c_str(),
				size.toString().c_str());
			return ProcessingStep::idle;
		}
	}

	// now build a target image
	if (nrawimages == 0) {
		throw std::runtime_error("no template size");
	}
	Image<double>	*_image
		= new Image<double>(rawimages[0]->out().getSize());
	imageptr = ImagePtr(_image);
	image = _image;
	image->fill(0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "common: create empty %s image",
		image->size().toString().c_str());

	// make the image availabe as preview
	_preview = PreviewAdapter::get(imageptr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "preview adapter created");

	// prepare images for medians, means and stddevs
	ImageSize	subsize = tileimagesize(size);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "aggregate image size: %s",
		subsize.toString().c_str());
	medians = new Image<double>(subsize);
	means = new Image<double>(subsize);
	stddevs = new Image<double>(subsize);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filling statistics images");
	for (unsigned int x = 0; x < subsize.width(); x += _spacing) {
		for (unsigned int y = 0; y < subsize.height(); y += _spacing) {
			filltile(x, y);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "statistics images ready");

	// Doing the pixel specific work.
	double	values[nrawimages];
	int	n;
	for (unsigned int x = 0; x < image->size().width(); x++) {
		for (unsigned int y = 0; y < image->size().height(); y++) {
debug(LOG_DEBUG, DEBUG_LOG, 0, "computing pixel value (%d,%d)", x, y);
			aggregates	a = aggr(x, y);
debug(LOG_DEBUG, DEBUG_LOG, 0, "mean = %f, stddev = %f", a.mean, a.stddev);
			get(x, y, values, n, a);
			// compute the average			
			if (n > 1) {
				image->writablepixel(x, y) = mean(values, n);
			} else {
				image->writablepixel(x, y)
				= std::numeric_limits<double>::has_quiet_NaN;
			}
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration pixels computed");

	// common work done
	return ProcessingStep::complete;
}

/**
 * \brief Compute the median of a multiset of doubles
 */
static double	median(const std::multiset<double> values) {
	std::multiset<double>::const_iterator	i = values.begin();
	std::advance(i, values.size() / 2);
	double	m = *i;
	if (0 == (values.size() % 2)) {
		m = (m + *++i) / 2;
	}
	return m;
}

// auxiliar class to compute aggregations
class aggregator {
	int _counter;
	double  _xsum;
	double  _x2sum;
public:
	int     counter() const { return _counter; }
	double mean() const {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "xsum = %f, counter = %d",
			_xsum, _counter);
		return _xsum / _counter;
	}
	double stddev() const { 
		double  m = mean();
		return sqrt((_x2sum / _counter - m * m) * _counter / (_counter - 1.));
	}
	aggregator() : _counter(0), _xsum(0), _x2sum(0) { }
	void    operator()(double x) {
//debug(LOG_DEBUG, DEBUG_LOG, 0, "add value  %f", 65535 * x);
		_xsum += x;
		_x2sum += x * x;
		_counter++;
	}
};


/**
 * \brief compute the aggregates for a tile
 *
 * Compute the averages for a tile. The values are taken from a slightly larger
 * piece of the image, like this:
 *
 *
 *        <------ width --------> <------- width ------->
 *       +-----------------------+-----------------------+
 *       |                       |                       |  ^
 *       |                       |                       |  |
 *       |       +---------------+---------------+       |  |
 *       |       |               |               |       |  |
 *       |       |               |               |       |  | width
 *       |       |               |               |       |  |
 *       |       |               |               |       |  |
 *       |       |               |               |       |  |
 *       |       |               | (xc,yc)       |       |  v
 *       +-------+---------------+---------------+-------+
 *       |       |<----grid----->|<----grid---^->|       |
 *       |       |               |            |  |       |
 *       |       |               |            |  |       |
 *       |       |               |       grid |  |       |
 *       |       |               |            |  |       |
 *       |       |               |            v  |       |
 *       |       +---------------+---------------+       |
 *       |                       |                       |
 *       |                       |                       |
 *       +-----------------------+-----------------------+
 *
 * 
 */
CalibrationProcessorStep::aggregates	CalibrationProcessorStep::tile(int xc, int yc) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "compute aggregates for tile centered at %d,%d", xc, yc);
	// compute the rectangle we want to use, the width must be a multiple
	// of _spacing to ensure that we get all points from the appropriate
	// subgrid
	int	width = _spacing * (_step + 8 / _spacing);

	// compute the rectangle we have to scan. At the boundary of the
	// image, we have to correct the computed minimum and maximum indices
	// to ensure we never try to access pixel values outside the image area
	int	minx = xc - width;
	while (minx < 0) { minx += _spacing; }
	int	maxx = xc + width;
	while (maxx >= (int)image->size().width()) { maxx -= _spacing; }
	int	miny = yc - width;
	while (miny < 0) { miny += _spacing; }
	int	maxy = yc + width;
	while (maxy >= (int)image->size().height()) { maxy -= _spacing; }
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d <= x <= %d (%d), %d <= y <= %d (%d)",
		minx, maxx, maxx - minx, miny, maxy, maxy - miny);

	// now scan the image rectangle for pixel values. We only take the
	// valid values, NaNs are ignored, and collect them in a multiset.
	// This way we automatically get them sorted. This makes computing
	// the medians more efficient
	std::multiset<double>	pixels;
	for (int i = 0; i < (int)nrawimages; i++) {
		for (int x = minx; x <= maxx; x += _spacing) {
			for (int y = miny; y < maxy; y += _spacing) {
				double	v = rawimages[i]->out().pixel(x, y);
				if (v == v) {
					pixels.insert(v);
				}
			}
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d values found", pixels.size());

	// compute median and averages
	aggregates	a;

	// first the median
	a.median = median(pixels);

	// now the sum of values and their squares
	std::multiset<double>::const_iterator	begin = pixels.begin();
	std::multiset<double>::const_iterator	end = pixels.begin();
	int	m = pixels.size() / 10;
	std::advance(begin, m);
	std::advance(end, pixels.size() - m - 1);
	aggregator	ag = std::for_each(begin, end, aggregator());

	// get the aggregates
	a.mean = ag.mean();
	a.stddev = ag.stddev();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "median = %f, mean = %f, stddev = %f",
		a.median, a.mean, a.stddev);

	// we are done
	return a;
}



/**
 * \brief compute all averages for a tile position
 *
 * If the grid spacing is 1, then this amounts to just a single computation.
 * If the grid spacing is 2, as should be used for RGB images, then we
 * compute four sets of aggregates, one for every RGGB subgrid.
 * \param x	x coordindate in tile coordinates
 * \param y	y coordindate in tile coordinates
 * \param grid	grid constant 
 */
void	CalibrationProcessorStep::filltile(int x, int y) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "processing tile @(%d,%d)", x, y);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image coordinates center tile: (%d,%d)",
		xi(x), yi(x));
	for (int dx = 0; dx < _spacing; dx++) {
		for (int dy = 0; dy < _spacing; dy++) {
			int	xx = xi(x) + dx;
			int	yy = xi(y) + dy;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "xx = %d, yy = %d",
				xx, yy);
			aggregates	a = tile(xx, yy);
			medians->writablepixel(xx, yy) = a.median;
			means->writablepixel(xx, yy) = a.mean;
			stddevs->writablepixel(xx, yy) = a.stddev;
		}
	}
}

/**
 * \brief get pixel values at a given point
 *
 * This method collects all the values at pixel position (x,y) that are not
 * too far away from the mean.
 */
void	CalibrationProcessorStep::get(unsigned int x, unsigned int y,
	double *values, int& n, const aggregates& a) const {
	n = 0;
	for (size_t i = 0; i < nrawimages; i++) {
		double	v = rawimages[i]->out().pixel(x, y);
		// ignore invalid pixels
		if (v != v) {
			continue;
		}

		// if a pixel value is too far away, ignore it as well
		if (a.improbable(v)) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "value %f improbable", v);
			continue;
		}

		// if a pixel value survives both checks, use it
		values[n++] = v;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d values found", n);
}

/**
 * \brief access to the calibration image
 */
const ConstImageAdapter<double>&	CalibrationProcessorStep::out() const {
	if (NULL == image) {
		throw std::runtime_error("no image available");
	}
	return *image;
}

/**
 * \brief get the aggregates representative for an image point
 */
CalibrationProcessorStep::aggregates	CalibrationProcessorStep::aggr(unsigned int x, unsigned int y) const {
	int	xa = _spacing * x / _step + x % _spacing;
	int	ya = _spacing * y / _step + y % _spacing;
	aggregates	result;
	result.median = medians->pixel(xa, ya);
	result.mean = means->pixel(xa, ya);
	result.stddev = stddevs->pixel(xa, ya);
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"aggregate(%u,%u) -> (%d,%d): median = %f, mean = %f, stddev = %f",
		x, y, xa, ya, result.median, result.mean, result.stddev);
	return result;
}

//////////////////////////////////////////////////////////////////////
// creating a dark image
//////////////////////////////////////////////////////////////////////
/**
 * \brief Work to construct dark images
 *
 * The common work method collects aggregates around grid points, then
 * this method computes averages from pixel values that are not too far
 * away from the averages. If there are not enough pixels to compute a
 * reasonable value, the pixel is set to NaN.
 */
ProcessingStep::state	DarkProcessorStep::do_work() {
	// common preparation work
	ProcessingStep::state	preparation = common_work();
	if (preparation != ProcessingStep::complete) {
		return preparation;
	}

	// cheat
	return ProcessingStep::complete;
}

//////////////////////////////////////////////////////////////////////
// creating a flat image
//////////////////////////////////////////////////////////////////////
/**
 * \brief Work to construct flat images
 */
ProcessingStep::state	FlatProcessorStep::do_work() {
	// common preparation work
	ProcessingStep::state	preparation = common_work();
	if (preparation != ProcessingStep::complete) {
		return preparation;
	}

	// compute the mean value of all pixels in the image
	double	m = astro::image::filter::mean(imageptr);

	// ensure that the average value is 1
	unsigned int	w = image->size().width();
	unsigned int	h = image->size().height();
	for (unsigned int x = 0; x < w; x++) {
		for (unsigned int y = 0; y < h; y++) {
			image->writablepixel(x, y) = image->pixel(x, y) / m;
		}
	}

	// cheat
	return ProcessingStep::complete;
}

} // namespace process
} // namespace astro
