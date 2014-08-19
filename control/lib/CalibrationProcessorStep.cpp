/*
 * CalibrationProcessorStep.cpp -- implementation of creators for calibration
 *                                 images
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <AstroDebug.h>

using namespace astro::adapter;

namespace astro {
namespace process {

/**
 * \brief Create a new calibration processor
 */
CalibrationProcessor::CalibrationProcessor(CalibrationImage::caltype t)
	: CalibrationImage(t) {
	rawimages = NULL;
	nrawimages = 0;
	_spacing = 1;
	medians = NULL;
	means = NULL;
	stddevs = NULL;
}

/**
 * \brief Destroy the calibration processor
 */
CalibrationProcessor::~CalibrationProcessor() {
	if (NULL != rawimages) {
		delete rawimages;
	}
}

/**
 * \brief Find all RawImage precursors
 *
 * get pointers to all the precursors of type ImageStep, others don't have
 * image data output, so they cannot be used to build calibration images
 */
size_t	CalibrationProcessor::getPrecursors() {
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
 * \brief Common work for both ClabrationProcessors
 *
 * This step essentially takes care of getting all the precursor images
 */
ProcessingStep::state	CalibrationProcessor::common_work() {
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
	int	step = _spacing * 12;
	ImageSize	subsize(_spacing * size.width() / step,
				_spacing * size.height() / step);
	medians = new Image<double>(subsize);
	means = new Image<double>(subsize);
	stddevs = new Image<double>(subsize);
	for (unsigned int x = step / 2; x < size.width(); x += step) {
		for (unsigned int y = step / 2; y < size.height(); x += step) {
			
		}
	}

	// common work done
	return ProcessingStep::complete;
}

/**
 * \brief compute the aggregates for a tile
 *
 * Compute the averages for a tile
 */
CalibrationProcessor::aggregates	CalibrationProcessor::tile(int xc, int yc, int step) {
	// compute the rectangle we want to use
	int	width = step + 8;
	width = _spacing * (width / (2 * _spacing));

	// compute the rectangle we have to scan
	int	minx = xc - width;
	int	maxx = xc + width;
	int	miny = yc - width;
	int	maxy = yc + width;

	// now scan the image for pixel values
	std::multiset<double>	pixels;
	int	w = image->size().width();
	int	h = image->size().height();
	for (int i = 0; i < (int)nrawimages; i++) {
		for (int x = minx; x <= maxx; x += _spacing) {
			for (int y = miny; y < maxy; y += _spacing) {
				if ((x < 0) || (x >= w) || (y < 0) || (y >= h)){
					continue;
				}
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
	std::multiset<double>::const_iterator	mit = pixels.begin();
	std::advance(mit, pixels.size() / 2);
	a.median = *mit;
	if (0 == (pixels.size() % 2)) {
		a.median = (a.median + *++mit) / 2;
	}

	// now the sum of values and their squares
	int	counter = 0;
	std::multiset<double>::const_iterator	begin = pixels.begin();
	std::multiset<double>::const_iterator	end = pixels.begin();
	int	m = pixels.size() / 10;
	std::advance(begin, m);
	std::advance(end, pixels.size() - m);
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
void	CalibrationProcessor::filltile(int x, int y, int step) {
	int	xs = _spacing * x / step;
	int	ys = _spacing * y / step;
	for (int dx = 0; dx < _spacing; dx++) {
		for (int dy = 0; dy < _spacing; dy++) {
			aggregates	a = tile(x + dx, y + dy, step);
			medians->writablepixel(xs + dx, ys + dy) = a.median;
			means->writablepixel(xs + dx, ys + dy) = a.mean;
			stddevs->writablepixel(xs + dx, ys + dy) = a.stddev;
		}
	}
}

/**
 * \brief get pixel values at a given point
 */
void	CalibrationProcessor::get(unsigned int x, unsigned int y,
	double *values, int& n) {
	n = 0;
	for (size_t i = 0; i < nrawimages; i++) {
		double	v = rawimages[i]->out().pixel(x, y);
		if (v == v) {
			values[n++] = v;
		}
	}
}

/**
 * \brief access to the calibration image
 */
const ConstImageAdapter<double>&	CalibrationProcessor::out() const {
	if (NULL == image) {
		throw std::runtime_error("no image available");
	}
	return *image;
}

//////////////////////////////////////////////////////////////////////
// creating a dark image
//////////////////////////////////////////////////////////////////////
/**
 * \brief Work to construct dark images
 */
ProcessingStep::state	DarkProcessor::do_work() {
	// common preparation work
	ProcessingStep::state	preparation = common_work();
	if (preparation != ProcessingStep::complete) {
		return preparation;
	}

	// create an adapter that is capable of computing the pixel values
	// for the dark

	// cheat
	return ProcessingStep::complete;
}

//////////////////////////////////////////////////////////////////////
// creating a flat image
//////////////////////////////////////////////////////////////////////
/**
 * \brief Work to construct flat images
 */
ProcessingStep::state	FlatProcessor::do_work() {
	// common preparation work
	ProcessingStep::state	preparation = common_work();
	if (preparation != ProcessingStep::complete) {
		return preparation;
	}

	// cheat
	return ProcessingStep::complete;
}

} // namespace process
} // namespace astro
