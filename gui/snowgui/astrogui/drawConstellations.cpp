/*
 * drawConstellations.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "SkyDrawing.h"
#include <AstroCoordinates.h>
#include <AstroDebug.h>
#include <ConstellationCatalog.h>

namespace snowgui {

/**
 * \brief Draw constellation lines
 */
void	SkyDrawing::drawConstellations(QPainter& painter) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw constellation lines");
	// set up the pen 
	QPen	pen(Qt::SolidLine);
	pen.setWidth(1);
	QColor	pink(255,0,204);
	pen.setColor(pink);
	painter.setPen(pen);

	// get the Constellations
	astro::catalog::ConstellationCatalogPtr	consts
		= astro::catalog::ConstellationCatalog::get();
	for (auto c = consts->begin(); c != consts->end(); c++) {
		// get the next constellation
		astro::catalog::ConstellationPtr	constellation = c->second;
		for (auto e = constellation->begin();
			e != constellation->end(); e++) {
			// go through all the edges
			drawLine(painter, e->from(), e->to());
		}
	}
}

} // namespace snowgui
