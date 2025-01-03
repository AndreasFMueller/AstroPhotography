/*
 * SkyStarThread.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <SkyDisplayWidget.cpp>
#include <AstroDebug.h>
#include <QThread>
#include <AstroCoordinates.h>

using namespace astro::catalog;

namespace snowgui {

/**
 * \brief Create the star retrieval thread
 */
SkyStarThread::SkyStarThread(QObject *parent, bool send_tile)
	: QThread(parent), _send_tile(send_tile) {
}

/**
 * \brief Destroy the star retrieval thread
 */
SkyStarThread::~SkyStarThread() {
}

/**
 * \brief Main method
 *
 * This method retrieves all stars from the catalog and emits them
 * as a signal, primarily for SkyDisplayWidget
 */
void	SkyStarThread::run() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "star retrieval started");
	try {
		CatalogPtr catalog = CatalogFactory::get();
		SkyWindow	windowall;
		MagnitudeRange  magrange(-30, 6);
		if (_send_tile) {
			StarTilePtr	startile = catalog->findTile(windowall,
							magrange);
			emit stars(startile);
		} else {
			Catalog::starsetptr	_stars
				= catalog->find(windowall, magrange);
			astro::Precession	precession;
			_stars = precess(precession, _stars);
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"star retrieval complete");
			emit stars(_stars);
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"stars sent to main thread");
		}
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no catalog available: %s", x.what());
	}
}

} // namespace snowgui
