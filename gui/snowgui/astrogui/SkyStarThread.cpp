/*
 * SkyStarThread.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <SkyDisplayWidget.cpp>
#include <AstroDebug.h>
#include <QThread>

using namespace astro::catalog;

namespace snowgui {

/**
 * \brief Create the star retrieval thread
 */
SkyStarThread::SkyStarThread(QObject *parent) : QThread(parent) {
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
	CatalogPtr catalog = CatalogFactory::get();
	SkyWindow	windowall;
	MagnitudeRange  magrange(-30, 6);
	Catalog::starsetptr	_stars = catalog->find(windowall, magrange);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "star retrieval complete");
	emit stars(_stars);
}

} // namespace snowgui
