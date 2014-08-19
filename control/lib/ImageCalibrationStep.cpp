/*
 * ImageCalibrationStep.cpp -- Calibration of raw images
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <AstroDebug.h>
#include <AstroFormat.h>

using namespace astro::image;
using namespace astro::adapter;

namespace astro {
namespace process {

/**
 * \brief Create an Image Calibration step
 */
ImageCalibration::ImageCalibration() {
	_image = NULL;
}

/**
 * \brief Destroy an Image Calibration step
 */
ImageCalibration::~ImageCalibration() {
	if (NULL != _image) {
		delete _image;
	}
}

/**
 * \brief Auxiliary function to search the precursors for a calibration image
 */
const CalibrationImage	*ImageCalibration::calimage(
				CalibrationImage::caltype t) const {
	ProcessingStep::steps::const_iterator	i
		= std::find_if(precursors().begin(), precursors().end(),
		[t](ProcessingStep *step) {
			CalibrationImage	*image
				= dynamic_cast<CalibrationImage *>(step);
			if (image == NULL) {
				return false;
			}
			return (t == image->type());
		}
	);
	if (i == precursors().end()) {
		throw std::runtime_error(
			stringprintf("no precursor of type %s found",
				CalibrationImage::caltypename(t).c_str()));
	}
	const CalibrationImage	*result
		= dynamic_cast<const CalibrationImage *>(*i);
	if (NULL == result) {
		std::runtime_error("precursor is not a clibration image");
	}
	return result;
}

/**
 * \brief Adapter to perform the calibration for full size images
 */
class CalibrationAdapter : public ConstImageAdapter<double> {
protected:
	const CalibrationImage *_dark;
	const CalibrationImage *_flat;
	const ConstImageAdapter<double>& _image;
	virtual double	darkpixel(unsigned int x, unsigned int y) const {
		return _dark->out().pixel(x, y);
	}
	virtual double	flatpixel(unsigned int x, unsigned int y) const {
		return _flat->out().pixel(x, y);
	}
public:
	CalibrationAdapter(
		const CalibrationImage *dark, const CalibrationImage *flat,
		const ConstImageAdapter<double>& image)
		: ConstImageAdapter<double>(image.getSize()),
		  _dark(dark), _flat(flat), _image(image) { }

	virtual double	pixel(unsigned int x, unsigned int y) const {
		double	value = _image.pixel(x, y);
		if (_dark) {
			value -= darkpixel(x, y);
		}
		if (_flat) {
			value /= flatpixel(x, y);
		}
		return value;
	}
};

/**
 * \brief Adapter to perform the calibration for subrectangles
 */
class WindowedCalibrationAdapter : public CalibrationAdapter {
	ImageRectangle	_window;
	virtual double	darkpixel(unsigned int x, unsigned int y) const {
		return _dark->out().pixel(_window.origin().x() + x,
			_window.origin().y() + y);
	}
	virtual double	flatpixel(unsigned int x, unsigned int y) const {
		return _flat->out().pixel(_window.origin().x() + x,
			_window.origin().y() + y);
	}
public:
	WindowedCalibrationAdapter(
		const CalibrationImage *dark, const CalibrationImage *flat,
		const ConstImageAdapter<double>& image,
		const ImageRectangle& window) 
		: CalibrationAdapter(dark, flat, image), _window(window) {
		// verify that the window fits inside the calibration image
		// retangles
		if (NULL != dark) {
			if (!dark->out().getSize().bounds(_window)) {
				throw std::runtime_error("window does not fit "
					"in dark");
			}
		}
		if (NULL != flat) {
			if (!flat->out().getSize().bounds(_window)) {
				throw std::runtime_error("window does not fit "
					"in flat");
			}
		}
	}
};

/**
 * \brief Do the actual work
 *
 * Image calibration needs a dark image an a flat image, which it looks
 * for among the precursors. It then calibrates the remaining image
 */
ProcessingStep::state	ImageCalibration::do_work() {
	// scan precursors for dark, flat and image to calibrate
	const CalibrationImage	*dark = NULL;
	const CalibrationImage	*flat = NULL;
	try {
		dark = calimage(CalibrationImage::DARK);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found a dark precursor: %s",
			dark->out().getSize().toString().c_str());
	} catch (...) { }
	try {
		flat = calimage(CalibrationImage::FLAT);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found a flat precursor: %s",
			flat->out().getSize().toString().c_str());
	} catch (...) { }

	// verify that the calibration images have the same size
	if ((NULL != dark) && (NULL != flat)) {
		if (dark->out().getSize() != flat->out().getSize()) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration image "
				"sizes don't match");
			throw std::runtime_error("calibration image sizes "
				"don't match");
		}
	}

	// find an image that is different
	ProcessingStep::steps::const_iterator	i
		= std::find_if(precursors().begin(), precursors().end(),
		[dark, flat](ProcessingStep *step) {
			if (NULL == dynamic_cast<ImageStep *>(step)) {
				return false;
			}
			return ((step != dark) && (step != flat));
		}
	);
	if (i == precursors().end()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no image to calibrate");
		return ProcessingStep::idle;
	}
	ImageStep	*image = dynamic_cast<ImageStep *>(*i);

	// ensure that the _image is properly removed
	if (NULL != _image) {
		delete _image;
		_image = NULL;
	}

	// now build the calibration adapter
	if (image->out().getSize() == dark->out().getSize()) {
		_image = new CalibrationAdapter(dark, flat,
			image->out());
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for subframe info");
		// this case can only be handled if the precursor is a raw
		// image (which will normally be the case) and has metadata
		// indicating that this is a subrectangle image
		RawImageFile	*raw = dynamic_cast<RawImageFile *>(image);
		if (NULL == raw) {
			std::string	msg("not a RawImageFile, "
				"cannot get subframe info");
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
			throw ProcessingStep::idle;
		}

		ImageRectangle	window = raw->subframe();
		_image = new WindowedCalibrationAdapter(dark, flat,
				image->out(), window);
	}
	return ProcessingStep::complete;
}

} // namespace process
} // namespace astro
