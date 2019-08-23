/*
 * DeepSkyRetriever.cpp -- separate thread to retrieve the stars
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <StarChartWidget.h>
#include <AstroDebug.h>

using namespace astro::catalog;

namespace snowgui {

/**
 * \brief Construct a star chart retriever thread
 */
DeepSkyRetriever::DeepSkyRetriever(QObject *parent) : QThread(parent) {
}

/**
 * \brief Work to do as the retriever
 */
void	DeepSkyRetriever::run() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieving deep sky objs in window %s",
		window().toString().c_str());
	try {
		DeepSkyCatalogFactory	factory;
		DeepSkyCatalogPtr catalog = factory.get(DeepSkyCatalogFactory::NGCIC);
		SkyWindow	windowall;
		astro::catalog::DeepSkyObjectSetPtr	deepskyobjects
			= catalog->find(windowall);
		//astro::Precession	precession;
		//deepskyobjects = precess(precession, deepskyobjects);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d objs found", deepskyobjects->size());
		emit deepskyReady(deepskyobjects);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "could not get NGC catalog: %s", x.what());
	}
}

} // namespace snowgui
