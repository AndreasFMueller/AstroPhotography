/*
 * NullTracker.cpp -- NullTracker implementation 
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>

namespace astro {
namespace guiding {

/**
 * \brief Offset-Function of the Null-Tracker
 *
 * This method always returns 0 as the offset
 */
Point	NullTracker::operator()(image::ImagePtr /* newimage */) {
	return Point(0,0);
}

} // namespace guiding
} // namespace astro
