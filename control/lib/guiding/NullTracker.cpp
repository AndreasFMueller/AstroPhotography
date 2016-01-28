/*
 * NullTracker.cpp -- NullTracker implementation 
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>

namespace astro {
namespace guiding {

Point	NullTracker::operator()(image::ImagePtr /* newimage */) {
	return Point(0,0);
}

} // namespace guiding
} // namespace astro
