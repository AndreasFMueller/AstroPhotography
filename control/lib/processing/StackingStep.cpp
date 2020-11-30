/*
 * StackingStep.cpp -- implement stacking process
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <AstroStacking.h>
#include <AstroImage.h>
#include <sstream>

using namespace astro::image;

namespace astro {
namespace process {

/**
 * \brief Create a new stacking step
 */
StackingStep::StackingStep(NodePaths& parent) : ImageStep(parent) {
	_patchsize = 256;
	_residual = 30;
	_searchradius = 10;
	_numberofstars = 20;
	_notransform = false;
	_usetriangles = false;
	_rigid = false;
	_rescale = true;	// rescale by default
}

#define do_rescale(Pixel)						\
{									\
	Image<Pixel >	*img = dynamic_cast<Image<Pixel >*>(&*image);	\
	if (img) {							\
		Pixel	x;						\
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s pixel", 		\
			demangle_string(x).c_str());			\
		for (int x = 0; x < w; x++) {				\
			for (int y = 0; y < h; y++) {			\
				Pixel	p = img->pixel(x, y);		\
				p = p * s;				\
				img->writablepixel(x, y) = p;		\
			}						\
		}							\
		return;							\
	}								\
}

/**
 * \brief Rescale an image with the factor given as second argument
 *
 * \param image		image to rescale
 * \param s		scaling factor
 */
void	StackingStep::rescale_image(ImagePtr image, double s) {
	int	w = image->size().width();
	int	h = image->size().height();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rescaling %dx%d image", w, h);
	do_rescale(unsigned char)
	do_rescale(unsigned short)
	do_rescale(unsigned int)
	do_rescale(unsigned long)
	do_rescale(float)
	do_rescale(double)
	do_rescale(RGB<unsigned char>)
	do_rescale(RGB<unsigned short>)
	do_rescale(RGB<unsigned int>)
	do_rescale(RGB<unsigned long>)
	do_rescale(RGB<float>)
	do_rescale(RGB<double>)
}

/**
 * \brief perform the stacking operation
 */
ProcessingStep::state	StackingStep::do_work() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stack %d images", precursors().size());
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %s base image",
		baseimageptr->size().toString().c_str());

	// create a stacker based on the base image
	astro::image::stacking::StackerPtr	stacker
		= astro::image::stacking::Stacker::get(baseimageptr);

	// set the parameters
	stacker->patchsize(_patchsize);
	stacker->residual(_residual);
	stacker->searchradius(_searchradius);
	stacker->numberofstars(_numberofstars);
	stacker->notransform(_notransform);
	stacker->usetriangles(_usetriangles);
	stacker->rigid(_rigid);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stacker created and parametrized");
	
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
			stacker->add(is->image(), is->transform());
			counter++;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d images added to the stacker",
		counter);

	// extract the result image
	_image = stacker->image();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s stacked extracted",
		_image->size().toString().c_str());

	// if rescaling is requested, do it now
	if (rescale()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "rescaling the image");
		rescale_image(_image, 1. / counter);
	}

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
