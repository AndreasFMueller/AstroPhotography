/*
 * LargeTracker.cpp -- a tracking method that trackes heavy objects
 *
 * (c) 2016 Prof Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <AstroFilter.h>
#include <memory>

using namespace astro::image::filter;

namespace astro {
namespace guiding {

/**
 * \brief Tracking the center of gravity of an image
 */
Point	LargeTracker::operator()(image::ImagePtr newimage) {
	ConstImageAdapter<double>	*a = adapter(newimage);
	std::shared_ptr<ConstImageAdapter<double> >	aptr(a);
	CGFilter	cgfilter(100);
	Point	cg = cgfilter(*a);
	Point	offset = cg - newimage->size().center();
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"found center of gravity %s, center offset %s",
		cg.toString().c_str(), offset.toString().c_str());
	return offset;
}

} // namespace guiding
} // namespace astro
