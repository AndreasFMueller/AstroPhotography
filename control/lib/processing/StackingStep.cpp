/*
 * StackingStep.cpp -- implement stacking process
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <AstroStacking.h>
#include <sstream>

namespace astro {
namespace process {

/**
 * \brief Create a new stacking step
 */
StackingStep::StackingStep() {
	_patchsize = 256;
	_searchradius = 10;
	_numberofstars = 20;
	_notransform = false;
}

/**
 * \brief perform the stacking operation
 */
ProcessingStep::state	StackingStep::do_work() {
	// get the base image
	if (!_baseimage) {
		std::string	msg = stringprintf("no base image");
		throw std::runtime_error(msg);
	}
	ImageStep	*bi = dynamic_cast<ImageStep*>(&*_baseimage);
	if (NULL == bi) {
		std::string	msg = stringprintf("%d is not an image step",
			_baseimage->id());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	ImagePtr	baseimageptr = bi->image();

	// create a stacker based on the base image
	astro::image::stacking::StackerPtr	stacker
		= astro::image::stacking::Stacker::get(baseimageptr);

	// set the parameters
	stacker->patchsize(_patchsize);
	stacker->searchradius(_searchradius);
	stacker->numberofstars(_numberofstars);
	stacker->notransform(_notransform);
	
	// add the precursor images (except the base image)
	int	counter =0;
	ProcessingStep::steps::const_iterator	i;
	for (i = precursors().begin(); i != precursors().end(); i++) {
		if (*i == _baseimage->id())
			continue;
		ProcessingStepPtr	next = ProcessingStep::byid(*i);
		ImageStep	*is = dynamic_cast<ImageStep*>(&*next);
		if (NULL == is) {
			std::string	msg = stringprintf("%d is not an image",
				*i);
		} else {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "add image '%s'(%d)", 
				next->name().c_str(), next->id());
			stacker->add(is->image());
			counter++;
		}
	}

	// extract the result image
	_image = stacker->image();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d images added to the stacker",
		counter);

	return ProcessingStep::complete;
}

/**
 * \brief Info about what this step does for verbose mode
 */
std::string	StackingStep::what() const {
	std::ostringstream	out;
	out << "stack images on base image '" << _baseimage->name();
	out << "'(" << _baseimage->id() << "):";
	ProcessingStep::steps::const_iterator	i;
	for (i = precursors().begin(); i != precursors().end(); i++) {
		if (*i == _baseimage->id())
			continue;
		ProcessingStepPtr	next = ProcessingStep::byid(*i);
		ImageStep	*is = dynamic_cast<ImageStep*>(&*next);
		if (NULL != is) {
			out << " " << next->id();
		}
	}
	return out.str();
}

} // namespace process
} // namespace astro
