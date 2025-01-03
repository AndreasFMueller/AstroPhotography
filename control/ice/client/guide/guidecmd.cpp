/*
 * guidecmd.cpp -- guiding related command implementations
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "guide.h"
#include <cstdlib>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <iostream>
#include <IceConversions.h>
#include "display.h"

namespace snowstar {
namespace app {
namespace snowguide {

/**
 * \brief Implementation of the guide command
 */
int	Guide::guide_command(GuiderPrx guider) {
	if ((star.x == 0) && (star.y == 0)) {
		debug(LOG_WARNING, DEBUG_LOG, 0,
			"warning: calibration star not set");
	}
	// make sure we have all the information we need
	if ((guideinterval < 0) || (guideinterval > 60)) {
		std::string	cause = astro::stringprintf(
			"bad guideinterval: %.3f", guideinterval);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw std::runtime_error(cause);
	}
	
	// get the guider
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"start guiding with intervals gp=%.1f ao=%.1f",
		guideinterval, aointerval);
	if (method != TrackerUNDEFINED) {
		guider->setTrackerMethod(method);
	}
	guider->startGuiding(guideinterval, aointerval, stepping);

	// we are done
	return EXIT_SUCCESS;
}

/**
 * \brief Stop the guider 
 */
int	Guide::stop_command(GuiderPrx guider) {
	GuiderState	state = guider->getState();
	if (state != GuiderGUIDING) {
		std::cerr << "not guiding" << std::endl;
		return EXIT_FAILURE;
	}
	guider->stopGuiding();
	return EXIT_SUCCESS;
}

/**
 * \brief Tracks command implementation
 *
 * The tracks command displays a list of tracks available. If the verbose
 * flag is set, then information about each track is also returned, i.e.
 * the number of points and the duration. This information requires that
 * the points be retrieved from the server as well. This is a little wasteful,
 * but the data size is still quite managable, and there does not seem to
 * be a performance issue from this.
 */
int	Guide::tracks_command(GuiderFactoryPrx guiderfactory,
		const std::string& instrument) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get tracks from remote server");
	idlist	l = guiderfactory->getTracks(instrument);
	std::cout << l.size() << " tracks" << std::endl;
	for (auto ptr = l.begin(); ptr != l.end(); ptr++) {
		int	id = *ptr;
		if (verbose) {
			// id
			std::cout << astro::stringprintf("%4d: ", id);
			TrackingHistory	history
				= guiderfactory->getTrackingHistory(id);

			// start time
			std::cout << astro::timeformat("%Y-%m-%d %H:%M ",
				converttime((double)history.timeago));
			// guider used
			std::cout << history.instrument;

			// points/duration
			if (history.points.size() > 1) {
				std::cout << astro::stringprintf(" %6d pts",
					history.points.size());
				std::cout << astro::stringprintf("  %6.0fsec", 
					history.points.begin()->timeago
					- history.points.rbegin()->timeago);
			}
		} else {	
			std::cout << id;
		}
		std::cout << std::endl;
	}
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the history command
 *
 * The tracking history is identified by the id. If the verbose flag is set,
 * then all the points of the tracking history are displayed.
 */
int	Guide::history_command(GuiderFactoryPrx guiderfactory, long historyid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieving history %d", historyid);
	TrackingHistory	history = guiderfactory->getTrackingHistory(historyid);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "track uses calibrations GP=%d AO=%d",
		history.guideportcalid, history.adaptiveopticscalid);
	if (history.points.size() == 0) {
		std::cout << "no tracking points found" << std::endl;
		return EXIT_SUCCESS;
	}
	TrackingHistory_display	display(guiderfactory, verbose, csv);
	display(history);

	return EXIT_SUCCESS;
}

int	Guide::history_command(GuiderFactoryPrx guiderfactory, long historyid,
		ControlType type) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieving history %d", historyid);
	TrackingHistory	history
		= guiderfactory->getTrackingHistoryType(historyid, type);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "track uses calibrations GP=%d AO=%d",
		history.guideportcalid, history.adaptiveopticscalid);
	if (history.points.size() == 0) {
		std::cout << "no tracking points found" << std::endl;
		return EXIT_SUCCESS;
	}
	TrackingHistory_display	display(guiderfactory, verbose, csv);
	display(history);

	return EXIT_SUCCESS;
}

/**
 * \brief Forget tracking histories
 */
int	Guide::forget_command(GuiderFactoryPrx guiderfactory,
		const std::list<int>& ids) {
	std::list<int>::const_iterator	i;
	for (i = ids.begin(); i != ids.end(); i++) {
		try {
			guiderfactory->deleteTrackingHistory(*i);
		} catch (const NotFound& x) {
			std::cerr << "cannot delete tracking history ";
			std::cerr << *i << ": ";
			std::cerr << x.cause << std::endl;
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}


} // namespace snowguide
} // namespace app
} // namespace snowstar
