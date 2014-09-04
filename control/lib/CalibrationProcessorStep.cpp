/*
 * CalibrationProcessorStep.cpp -- implementation of creators for calibration
 *                                 images
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <AstroDebug.h>
#include <AstroFilterfunc.h>
#include <AstroFormat.h>
#include <numeric>

using namespace astro::adapter;

namespace astro {
namespace process {

//////////////////////////////////////////////////////////////////////
// some auxiliary functions
//////////////////////////////////////////////////////////////////////

/**
 * \brief Compute the median of a multiset of doubles
 */
static double	median(const std::multiset<double>& values) {
	if (values.size() == 0) {
		return std::numeric_limits<double>::quiet_NaN();
	}
	std::multiset<double>::const_iterator	i = values.begin();
	std::advance(i, values.size() / 2);
	double	m = *i;
	if (0 == (values.size() % 2)) {
		m = (m + *++i) / 2;
	}
	return m;
}

static double	mean(const std::multiset<double>& values) {
	if (values.size() == 0) {
		return std::numeric_limits<double>::quiet_NaN();
	}
	double	sum = std::accumulate(values.begin(), values.end(), (double)0.);
	return sum/values.size();
}

// auxiliary class to compute aggregations
class aggregator {
	int _counter;
	double  _xsum;
	double  _x2sum;
public:
	int     counter() const { return _counter; }
	double mean() const {
		return _xsum / _counter;
	}
	double stddev() const { 
		double  m = mean();
		return sqrt((_x2sum / _counter - m * m) * _counter / (_counter - 1.));
	}
private:
	double	_maxoffset;
	double	_median;
public:
	aggregator(double median, double maxoffset)
		: _counter(0), _xsum(0), _x2sum(0),
		  _maxoffset(maxoffset), _median(median) { }
	void    operator()(double x) {
		if ((_maxoffset > 0) && (fabs(x - _median) > _maxoffset)) {
			return;
		}
		_xsum += x;
		_x2sum += x * x;
		_counter++;
	}
};

/**
 * \brief Auxiliary class to build values
 */
class value_constructor {
	const CalibrationProcessorStep::aggregates&	_tile;
	double	_tolerance;
	std::multiset<double>	values;
public:
	int	count() const { return values.size(); }
	int	badprecursors;
	int	improbablevalues;
public:
	value_constructor(const CalibrationProcessorStep::aggregates& tile,
		double tolerance)
		: _tile(tile), _tolerance(tolerance) {
		badprecursors = 0;
		improbablevalues = 0;
	}

	/**
 	 * \brief add a value if it sattisfies some conditions
 	 */
	void	operator()(const double v) {
		// ignore invalid pixels (nan values)
		if (v != v) {
			badprecursors++;
			return;
		}

		// if a pixel value is too far away, ignore it as well
		if (_tile.improbable(v, _tolerance)) {
			improbablevalues++;
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"value %f improbable", v);
			return;
		}

		// if a pixel value survives both checks, use it
		values.insert(v);
	}

	/**
	 * \brief compute mean value
	 */
	double	mean() const {
		return astro::process::mean(values);
	}

	/**
	 * \brief compute median value
	 */
	double	median() const {
		return astro::process::median(values);
	}
};


//////////////////////////////////////////////////////////////////////
// constructor/destructor
//////////////////////////////////////////////////////////////////////

/**
 * \brief Create a new calibration processor
 */
CalibrationProcessorStep::CalibrationProcessorStep(CalibrationImageStep::caltype t)
	: CalibrationImageStep(t) {
	rawimages = NULL;
	nrawimages = 0;
	_spacing = 1;
	_step = 10;
	_tolerance = 3.;
	_maxoffset = 0.;
	_margin = 0.1;
	_method = mean_method;
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

//////////////////////////////////////////////////////////////////////
// setters enforce that _step is always a multiple of _spacing
//////////////////////////////////////////////////////////////////////
/**
 * \brief Set the spacing of the subgrid for color pixels
 *
 * The value can only be set to a factor of _step. If s does not
 * devide the current value of _step, _step first has to be set to a value
 * that devides both the current and the future spacing value.
 */
void	CalibrationProcessorStep::spacing(int s) {
	if (_step % s) {
		std::string	msg = stringprintf("_step=%d not a multiple "
			"of %d", _step, s);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	_spacing = s;
}

/**
 * \brief Set the half grid constant of the tile centers
 */
void	CalibrationProcessorStep::step(int s) {
	if (s % _spacing) {
		std::string	msg = stringprintf("_spacing=%d does not divide"
			" %d", _spacing, s);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	_step = s;
}

/**
 * \brief Set step and spacing at the same time
 *
 * This method can be used when the new spacing and the old step setting
 * are incompatible, or vice versa.
 */
void	CalibrationProcessorStep::setStepAndSpacing(int newstep,
		int newspacing) {
	if (newstep % newspacing) {
		std::string	msg = stringprintf("spacing %d does not divide "
			"step %d", newspacing, newstep);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
}

/**
 * \brief The setter for the tolerance enforces a positive tolerance value
 */
void	CalibrationProcessorStep::tolerance(double t) {
	if (t <= 0) {
		std::string	msg = stringprintf("%f <= 0 is invalid as "
			"tolerance value", t);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	_tolerance = t;
}

//////////////////////////////////////////////////////////////////////
// tile coordinate conversions
//////////////////////////////////////////////////////////////////////
/**
 * \brief Get tile x coordinate from image coordinates
 */
short	CalibrationProcessorStep::xt(int x) const {
	return _spacing * (x / (2 * _step));
}

/**
 * \brief Get tile y coordinate from image coordinates
 */
short	CalibrationProcessorStep::yt(int y) const {
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
int	CalibrationProcessorStep::xi(short x) const {
	return _step * (1 + 2 * x);
}

/**
 * \brief Image y coordinate from tile coordinate
 */
int	CalibrationProcessorStep::yi(short y) const {
	return _step * (1 + 2 * y);
}

/**
 * \brief Get size for the tile images for aggregates
 */
ImageSize	CalibrationProcessorStep::tileimagesize(const ImageSize& size) const {
	int	s = _step * 2;
	return ImageSize(_spacing * (1 + ((size.width() - _step) / s)),
			_spacing * (1 + ((size.height() - _step) / s)));
}

/**
 * \brief Find all RawImage precursors
 *
 * Get pointers to all the precursors of type ImageStep, others don't have
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
 * \brief Common work for both ClabrationProcessors
 *
 * This step essentially takes care of getting all the precursor images
 */
ProcessingStep::state	CalibrationProcessorStep::common_work() {
	// first find pointers to all the precursor images
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
	for (short x = 0; x < (short)subsize.width(); x += _spacing) {
		for (short y = 0; y < (short)subsize.height(); y += _spacing) {
			filltile(x, y);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "statistics images ready");

	// Doing the pixel specific work.
	for (unsigned int x = 0; x < image->size().width(); x++) {
		for (unsigned int y = 0; y < image->size().height(); y++) {
			value_constructor	c(aggr(x, y), _tolerance);
			for (unsigned int i = 0; i < nrawimages; i++) {
				double	v = rawimages[i]->out().pixel(x, y);
				c(v);
			}
			double	pixelvalue = 0.;
			switch (_method) {
			case mean_method:
				pixelvalue = c.mean();
				break;
			case median_method:
				pixelvalue = c.median();
				break;
			}
			image->writablepixel(x, y) = pixelvalue;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration pixels computed");

	// common work done
	return ProcessingStep::complete;
}

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
 *       |       |               | (xb,yb)       |       |  v
 *       +-------+---------------+---------------+-------+
 *       |       |<---_step----->|<---_step---^->|       |
 *       |       |               |            |  |       |
 *       |       |               |            |  |       |
 *       |       |               |      _step |  |       |
 *       |       |               |            |  |       |
 *       |       |               |            v  |       |
 *       |       +---------------+---------------+       |
 *       |                       |                       |
 *       |                       |                       |
 *       +-----------------------+-----------------------+
 *
 * (xb,yb) is the base point of the tile subgrid, it may be offset from the
 * tile center if _spacing is > 1, but by at most _spacing-1 in each direction.
 */
CalibrationProcessorStep::aggregates	CalibrationProcessorStep::tile(int xb,
						int yb) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "compute aggregates for tile "
		"centered at %d,%d", xb, yb);
	// compute the rectangle we want to use, the width must be a multiple
	// of _spacing to ensure that we get all points from the appropriate
	// subgrid
	int	width = _step + _spacing * (_step / 2);

	// compute the rectangle we have to scan. At the boundary of the
	// image, we have to correct the computed minimum and maximum indices
	// to ensure we never try to access pixel values outside the image area
	int	minx = xb - width;
	while (minx < 0) { minx += _spacing; }
	int	maxx = xb + width;
	while (maxx >= (int)image->size().width()) { maxx -= _spacing; }
	int	miny = yb - width;
	while (miny < 0) { miny += _spacing; }
	int	maxy = yb + width;
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d values to aggregate",
		pixels.size());

	// compute median and averages
	aggregates	a;

	// first the median
	a.median = median(pixels);

	// now the sum of values and their squares, we leave out the top
	// and bottom margin as determined by the value of _margin.
	// by leaving out the extremes, we get less noisy mean and median
	// values.
	std::multiset<double>::const_iterator	begin = pixels.begin();
	std::multiset<double>::const_iterator	end = pixels.begin();
	int	m = pixels.size() * _margin;
	std::advance(begin, m);
	std::advance(end, pixels.size() - m);
	aggregator	ag = std::for_each(begin, end,
				aggregator(a.median, _maxoffset));

	// get the aggregates
	a.mean = ag.mean();
	a.stddev = ag.stddev();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "(%d,%d): median = %f, "
		"mean = %f, stddev = %f", xb, yb, a.median, a.mean, a.stddev);

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
 */
void	CalibrationProcessorStep::filltile(short x, short y) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "processing tile @(%d,%d)", x, y);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image coordinates center tile: (%d,%d)",
		xi(x), yi(x));
	for (short dx = 0; dx < _spacing; dx++) {
		for (short dy = 0; dy < _spacing; dy++) {
			int	xx = x + dx;
			int	yy = y + dy;
			aggregates	a = tile(xi(x) + dx, yi(y) + dy);
			medians->writablepixel(xx, yy) = a.median;
			means->writablepixel(xx, yy) = a.mean;
			stddevs->writablepixel(xx, yy) = a.stddev;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "stored aggregates: "
				"median=%f, mean=%f, stddev=%f @ (%d,%d)",
				medians->pixel(xx, yy), means->pixel(xx, yy),
				stddevs->pixel(xx, yy), xx, yy);
		}
	}
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
 *
 * This method retrieves the representative aggregates from the aggregate 
 * images. For this purpose, it first has to compute the coordinates of the
 * tile,
 */
CalibrationProcessorStep::aggregates	CalibrationProcessorStep::aggr(
		unsigned int x, unsigned int y) const {
	// we need to know the tile coordinates for these pixel coordinates
	short	tilex = xt(x);
	short	tiley = yt(y);

	// if _spacing is > 1, we need to find out to which subpixel the
	// coordinates refer
	short	dx = x % _spacing;
	short	dy = y % _spacing;

	// now we can compute the coordinates for the aggregate pixel
	short	xa = tilex + dx;
	short	ya = tiley + dy;

	// now retrieve the aggregates from the aggregate images
	aggregates	result;
	result.median = medians->pixel(xa, ya);
	result.mean = means->pixel(xa, ya);
	result.stddev = stddevs->pixel(xa, ya);
	if (debuglevel > LOG_DEBUG) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"aggregate(%u,%u) -> (%hd,%hd): median = %f, "
			"mean = %f, stddev = %f", x, y, xa, ya,
			result.median, result.mean, result.stddev);
	}
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

	// done
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "dividing by %f", m);

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
