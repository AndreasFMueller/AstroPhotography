/*
 * FlatImageStep.cpp -- implement the FlatImageStep class
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperwil
 */
#include <AstroProcess.h>
#include <AstroCalibration.h>
#include <sstream>

namespace astro {
namespace process {

/**
 * \brief Create a FlatImageStep instance
 */
FlatImageStep::FlatImageStep() {
}

/**
 * \brief Perform the work to create a flat image
 */
ProcessingStep::state	FlatImageStep::do_work() {
	// find the pointer to the dark image
	int	darkid = -1;
	if (_dark) {
		darkid = _dark->id();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "dark id = %d", darkid);

	// build the list of precursor images
	ImageSequence	images;
	std::for_each(precursors().begin(), precursors().end(),
		[&images,darkid](int precursorid) mutable {
			ProcessingStepPtr p = ProcessingStep::byid(precursorid);
			if (!p) {
				// ignore steps that dont exist
			}
			ImageStep       *j = dynamic_cast<ImageStep*>(&*p);
			if (NULL == j) {
				return;
			}
			// ignore the dark image as a precursor
			if (j->id() == darkid) {
				return;
			}
			images.push_back(j->image());
		}
	);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d images", images.size());

	// extract the dark image
	ImagePtr	darkimage;
	if (darkid >= 0) {
		ImageStep	*imagestep = dynamic_cast<ImageStep*>(&*_dark);
		if (imagestep) {
			darkimage = imagestep->image();
			debug(LOG_DEBUG, DEBUG_LOG, 0, "found dark image %s",
				darkimage->size().toString().c_str());
		}
	}

	// use the FlatFrameFactory to create a new flat image
	astro::calibration::FlatFrameFactory	fff;
	_image = fff(images, darkimage);

	// remember the current time
	time_t	now;
	time(&now);
	ProcessingStep::when(now);

	// return completion state
	return ProcessingStep::complete;
}

std::string	FlatImageStep::what() const {
	std::ostringstream	out;
	out << "build flat from ";
	int	images = precursors().size() - ((_dark) ? 1 : 0);
	out << images << " images";
	if (_dark) {
		out << ", dark='" << _dark->name() << "'(" << _dark->id() << ")";
	}
	return out.str();
}

} // namespace process
} // namespace astro
