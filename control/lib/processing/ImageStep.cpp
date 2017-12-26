/*
 * ImageStep.cpp -- processing steps that represent image
 *
 * This was historically the first piece of the project that used lambdas
 * in an essential way
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperwil
 */
#include <AstroProcess.h>
#include <algorithm>
#include <includes.h>
#include <AstroDebug.h>

using namespace astro::adapter;

namespace astro {
namespace process {

/**
 * \brief Retrieve the unique precursor image
 *
 * If there is not exactly one precursor image, throw an exception,
 * otherwise return the unique precursor image
 */
ImagePtr	ImageStep::precursorimage() {
	// get the image from the precursor
	ImageSequence	p = precursorimages();
	if (p.size() != 1) {
		std::string	msg = stringprintf("wrong number of precursor "
			"images: %d != 1", p.size());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	ImagePtr	precursorimage = *p.begin();
	return precursorimage;
}

} // namespace process
} // namespace astro
