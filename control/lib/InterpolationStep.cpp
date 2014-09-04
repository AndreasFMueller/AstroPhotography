/*
 * InterpolationStep.cpp -- step that performs interpolation for 
 *                               bad pixels
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProcess.h>

namespace astro {
namespace process {

//////////////////////////////////////////////////////////////////////
// adapter to do the interpolation
//////////////////////////////////////////////////////////////////////
class InterpolationAdapter : public ConstImageAdapter<double> {
	const ConstImageAdapter<double>&	_image;
	unsigned int	_spacing;
public:
	InterpolationAdapter(const ConstImageAdapter<double>& image,
		unsigned int spacing)
		: ConstImageAdapter<double>(image.getSize()), _image(image),
		  _spacing(spacing) {
	}

	double	pixel(unsigned int x, unsigned int y) const {
		double	v = _image.pixel(x, y);
		if (v == v) {
			return v;
		}
		double	sum = 0;
		int	n = 0;
		if (x >= _spacing) {
			v = _image.pixel(x - _spacing, y);
			if (v == v) {
				sum += v;
				n++;
			} else {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"bad left pixel value");
			}
		}
		if (x + _spacing < _image.getSize().width()) {
			v = _image.pixel(x + _spacing, y);
			if (v == v) {
				sum += v;
				n++;
			} else {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"bad right pixel value");
			}
		}
		if (y >= _spacing) {
			v = _image.pixel(x, y - _spacing);
			if (v == v) {
				sum += v;
				n++;
			} else {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"bad below pixel value");
			}
		}
		if (y + _spacing < _image.getSize().height()) {
			v = _image.pixel(x, y + _spacing);
			if (v == v) {
				sum += v;
				n++;
			} else {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"bad above pixel value");
			}
		}
		if (n == 0) {
			return std::numeric_limits<double>::quiet_NaN();
		}
		if (n != 4) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"only %d for interpolation at (%u,%u)",
				n, x, y);
		}
		return sum / n;
	}
};

/**
 * \brief construct an interpolation step
 */
InterpolationStep::InterpolationStep(int spacing)
	: _spacing(spacing) {
}

/**
 * \brief Work method for pixel interpolation
 *
 * Pixel interpolation typically happens on the fly, as there are only 
 * very few pixels to interpolate. Thus the only work to do is to find
 * the precursor image and create an interpolating adapter.
 */
ProcessingStep::state	InterpolationStep::do_work() {
	// find the single precursor, which also must be an image step
	if (precursors().size() == 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"not precursor for interpolation");
		return ProcessingStep::idle;
	}
	ProcessingStep	*step = *precursors().begin();
	ImageStep	*imagestep = dynamic_cast<ImageStep *>(step);
	if (NULL == imagestep) {
		throw std::runtime_error("precursor is not an image step");
	}

	// take the output from the precursor to buil the interpolation
	// adapter
	_out = outPtr(new InterpolationAdapter(imagestep->out(), _spacing));

	// that's it
	return ProcessingStep::complete;
}

} // namespace process
} // namespace stro
