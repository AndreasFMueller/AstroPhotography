/*
 * VCurveFocusWork.cpp -- implement the work finding the focus using a V-Curve
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <FocusWork.h>
#include <AstroDebug.h>
#include <limits>
#include <AstroFormat.h>
#include <FocusCompute.h>
#include <AstroFilterfunc.h>

using namespace astro::image::filter;

namespace astro {
namespace focusing {

class FWHM2Evaluator : public FocusEvaluator {
	ImagePoint	_center;
	double	_radius;
public:
	FWHM2Evaluator(const ImagePoint& center, double radius = 20)
		: _center(center), _radius(radius) { }
	virtual double	operator()(const ImagePtr image) {
		double  fwhm = focusFWHM2(image, _center, _radius);
		return fwhm;
	}
};

/**
 * \brief Main function of the Focusing process
 */
void	VCurveFocusWork::main(astro::thread::Thread<FocusWork>& thread) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start focusing work");
	if (!complete()) {
		focusingstatus(Focusing::FAILED);
		throw std::runtime_error("focuser not completely specified");
	}

	FocusCompute	fc;

	// determine how many intermediate steps we want to access

	if (min() < focuser()->min()) {
		throw std::runtime_error("minimum too small");
	}

	// based on the exposure specification, build an evaluator
	ImageSize	size = exposure().frame.size();
	int	radius = std::min(size.width(), size.height()) / 2;
	FWHM2Evaluator	evaluator(size.center(), radius);

	unsigned long	delta = max() - min();
	for (int i = 0; i < steps(); i++) {
		// compute new position
		unsigned short	position = min() + (i * delta) / (steps() - 1);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "measuring position %hu",
			position);

		// move to new position
		focusingstatus(Focusing::MOVING);
		focuser()->moveto(position);
		
		// get an image from the Ccd
		focusingstatus(Focusing::MEASURING);
		ccd()->startExposure(exposure());
		ccd()->wait();
		ImagePtr	image = ccd()->getImage();
		
		// turn the image into a value
		FWHMInfo	fwhminfo = focusFWHM2_extended(image,
					size.center(), radius);
		double	value = fwhminfo.radius;

		// add the new value 
		fc.insert(std::pair<unsigned short, double>(position, value));

		// send the callback data
		callback(combine(image, fwhminfo), value);
	}

	// compute the best focus position
	double	focusposition = fc.focus();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "optimal focus position: %f",
		focusposition);

	// plausibility check for the position
	if (!((focusposition >= min()) && (focusposition <= max()))) {
		focusingstatus(Focusing::FAILED);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "focusing failed");
		return;
	}

	// move to the focus position
	unsigned short	targetposition = focusposition;
	focusingstatus(Focusing::MOVING);
	focuser()->moveto(targetposition);
	focusingstatus(Focusing::FOCUSED);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "target position reached");
}

/**
 * \brief Combine image, mask and center into a color image
 */
ImagePtr	VCurveFocusWork::combine(ImagePtr image, FWHMInfo& fwhminfo) {
	return image;
}

} // namespace focusing
} // namespace astro
