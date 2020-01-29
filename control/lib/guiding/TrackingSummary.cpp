/*
 * TrackingSummary.cpp -- summary information about tracking
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <time.h>
#include <AstroGateway.h>

namespace astro {
namespace guiding {

/**
 * \brief Construct a new Tracking summary object
 */
TrackingSummary::TrackingSummary(const std::string& instrument)
	: descriptor(instrument) {
	trackingid = -1;
}

void	TrackingSummary::addPoint(const Point& offset) {
	BasicSummary::addPoint(offset);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding offset to taskupate");
	gateway::Gateway::update(descriptor.instrument(), offset);
}

} // namespace guiding
} // namespace astro
