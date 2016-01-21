/*
 * TrackingSummary.cpp -- summary information about tracking
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <time.h>

namespace astro {
namespace guiding {

/**
 * \brief Construct a new Tracking summary object
 */
TrackingSummary::TrackingSummary(const std::string& name,
		const std::string& instrument, const std::string& ccd,
		const std::string& guiderport,
		const std::string& adaptiveoptics)
	: descriptor(name, instrument, ccd, guiderport, adaptiveoptics) {
}

} // namespace guiding
} // namespace astro
