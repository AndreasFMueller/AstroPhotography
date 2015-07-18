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
#include <AstroFilter.h>
#include <AstroAdapter.h>

using namespace astro::image::filter;
using namespace astro::adapter;

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
void	VCurveFocusWork::main(astro::thread::Thread<FocusWork>& /* thread */) {
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
	ImageSize	size = exposure().size();
	int	radius = std::min(size.width(), size.height()) / 2;
	FWHM2Evaluator	evaluator(size.center(), radius);

	unsigned long	delta = max() - min();
	for (int i = 0; i < steps(); i++) {
		// compute new position
		unsigned short	position = min() + (i * delta) / (steps() - 1);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "measuring position %hu",
			position);

		// move to new position
		moveto(position);
		
		// get an image from the Ccd
		focusingstatus(Focusing::MEASURING);
		ccd()->startExposure(exposure());
		usleep(1000000 * exposure().exposuretime());
		ccd()->wait();
		ImagePtr	image = ccd()->getImage();
		
		// turn the image into a value
		FWHMInfo	fwhminfo = focusFWHM2_extended(image,
					size.center(), radius);
		double	value = fwhminfo.radius;

		// add the new value 
		fc.insert(std::pair<unsigned short, double>(position, value));

		// send the callback data
		callback(combine(image, fwhminfo), position, value);
	}

	// compute the best focus position
	double	focusposition = 0;
	try {
		focusposition = fc.focus();
	} catch (std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no optimal focus position: %s",
			x.what());
		focusingstatus(Focusing::FAILED);
		return;
	}
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
	moveto(targetposition);
	focusingstatus(Focusing::FOCUSED);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "target position reached");
}

/**
 * \brief Combine image, mask and center into a color image
 */
ImagePtr	VCurveFocusWork::combine(ImagePtr image, FWHMInfo& fwhminfo) {
	// first build the red channel from the mask
	Image<unsigned char>	*red
		= dynamic_cast<Image<unsigned char> *>(&*fwhminfo.mask);
	if (NULL == red) {
		throw std::logic_error("internal error, mask has not 8bit pixel type");
	}

	// then build the green channel from the original image
	Image<unsigned char>	*green = FocusWork::green(image);
	ImagePtr	greenptr(green);

	// create the blue image
	CrosshairAdapter<unsigned char>	crosshair(image->size(), fwhminfo.maxpoint, 20);
	CircleAdapter<unsigned char>	circle(image->size(), fwhminfo.center,
						fwhminfo.radius);
	MaxAdapter<unsigned char>	blue(crosshair, circle);

	// now use a combination adapter to put all these images together
	// into a single color image
	CombinationAdapter<unsigned char>	combinator(*red, *green, blue);
	Image<RGB<unsigned char> >	*result
		= new Image<RGB<unsigned char> >(combinator);

	return ImagePtr(result);
}

} // namespace focusing
} // namespace astro
