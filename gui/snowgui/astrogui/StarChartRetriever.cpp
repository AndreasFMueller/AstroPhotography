/*
 * StarChartRetriever.cpp -- separate thread to retrieve the stars
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "StarChartWidget.h"
#include <AstroDebug.h>

using namespace astro::catalog;

namespace snowgui {

/**
 * \brief Construct a star chart retriever thread
 */
StarChartRetriever::StarChartRetriever(QObject *parent) : QThread(parent) {
}

/**
 * \brief Work to do as the retriever
 */
void	StarChartRetriever::run() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieving stars in window %s", 
		window().toString().c_str());
	CatalogPtr catalog = CatalogFactory::get();
        MagnitudeRange  magrange(-30, limit_magnitude());
        astro::catalog::Catalog::starsetptr	stars
		= catalog->find(window(), magrange);
	astro::Precession	precession;
	stars = precess(precession, stars);
        debug(LOG_DEBUG, DEBUG_LOG, 0, "%d stars found", stars->size());
	emit starsReady(stars);
}

} // namespace snowgui
