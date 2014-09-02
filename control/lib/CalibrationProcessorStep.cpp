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
 * \brief Find all RawImage precursors
 *
 * get pointers to all the precursors of type ImageStep, others don't have
 * image data output, so they cannot be used to build calibration images
 */
size_t	CalibrationProcessorStep::getPrecursors() {
	typedef ImageStep *ImageStepPtr;
	rawimages = new ImageStepPtr[precursors().size()];
	nrawimages = 0;

	// the lambda below cannot capture members, so we have to capture
	// local variables
	ImageStep	**ri = rawimages;
	int	n = 0;
	std::for_each(precursors().begin(), precursors().end(),
		[ri,n](ProcessingStep *step) mutable {
			ImageStep	*f = dynamic_cast<ImageStep *>(step);
			if (NULL != f) {
				ri[n++] = f;
			}
		}
	);
	nrawimages = n;

	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d raw images");
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create empty %s image",
		image->size().toString().c_str());

	// make the image availabe as preview
	_preview = PreviewAdapter::get(imageptr);

	// prepare images for medians, means and stddevs
	int	grid = _spacing * _step;
	ImageSize	subsize(_spacing * size.width() / grid,
				_spacing * size.height() / grid);
	medians = new Image<double>(subsize);
	means = new Image<double>(subsize);
	stddevs = new Image<double>(subsize);
	for (unsigned int x = grid; x < size.width(); x += 2 * grid) {
		for (unsigned int y = grid; y < size.height(); x += 2 * grid) {
			filltile(x, y, grid);
		}
	}

	// Doing the pixel specific work.
	double	values[nrawimages];
	int	n;
	for (unsigned int x = 0; x < image->size().width(); x++) {
		for (unsigned int y = 0; y < image->size().height(); y++) {
			aggregates	a = aggr(x, y);
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
CalibrationProcessorStep::aggregates	CalibrationProcessorStep::tile(int xc, int yc, int grid) {
	// compute the rectangle we want to use, the width must be a multiple
	// of _spacing to ensure that we get all points from the appropriate
	// subgrid
	int	width = grid + _spacing * (8 / _spacing);

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

	// compute median and averages
	aggregates	a;
	a.median = 0; a.mean = 0; a.stddev = 0;

	// first the median
	a.median = median(pixels);

	// now the sum of values and their squares
	int	counter = 0;
	std::multiset<double>::const_iterator	begin = pixels.begin();
	std::multiset<double>::const_iterator	end = pixels.begin();
	int	m = pixels.size() / 10;
	std::advance(begin, m);
	std::advance(end, pixels.size() - m - 1);
	std::for_each(begin, end,
		[a,counter](double x) mutable {
			a.mean += x;
			a.stddev += x + x;
			counter++;
		}
	);

	// compute standard mean and standard deviaton from this
	a.mean = a.mean / counter;
	a.stddev = (a.stddev / counter - a.mean * a.mean)
		* counter / (counter - 1.);

	// we are done
	return a;
}



/**
 * \brief compute all averages for a tile position
 *
 * If the grid spacing is 1, then this amounts to just a single computation.
 * If the grid spacing is 2, as should be used for RGB images, then we
 * compute four sets of aggregates, one for every RGGB subgrid.
 */
void	CalibrationProcessorStep::filltile(int x, int y, int grid) {
	int	xs = _spacing * x / (2 * grid);
	int	ys = _spacing * y / (2 * grid);
	for (int dx = 0; dx < _spacing; dx++) {
		for (int dy = 0; dy < _spacing; dy++) {
			aggregates	a = tile(x + dx, y + dy, grid);
			medians->writablepixel(xs + dx, ys + dy) = a.median;
			means->writablepixel(xs + dx, ys + dy) = a.mean;
			stddevs->writablepixel(xs + dx, ys + dy) = a.stddev;
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
			continue;
		}

		// if a pixel value survives both checks, use it
		values[n++] = v;
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
 */
CalibrationProcessorStep::aggregates	CalibrationProcessorStep::aggr(unsigned int x, unsigned int y) const {
	aggregates	result;
	int	xa = _spacing * x / _step + x % _spacing;
	int	ya = _spacing * y / _step + y % _spacing;
	result.median = medians->pixel(xa, ya);
	result.mean = means->pixel(xa, ya);
	result.stddev = stddevs->pixel(xa, ya);
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
