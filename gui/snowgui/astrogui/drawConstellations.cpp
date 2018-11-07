/*
 * drawConstellations.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "SkyDrawing.h"
#include <AstroCoordinates.h>
#include <AstroDebug.h>

namespace snowgui {

#include "constellations.h"

/**
 * \brief Draw constellation lines
 */
void	SkyDrawing::drawConstellations(QPainter& painter) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw constellation lines");
	// set up the pen 
	QPen	pen(Qt::SolidLine);
	pen.setWidth(1);
	QColor	pink(255,0,204);
	pen.setColor(pink);
	painter.setPen(pen);

	// start processing the data
	for (int i = 0; i < constellation_size - 1; i++) {
		// skip nulls
		if ((constellation_points[i].name == NULL) ||
			(constellation_points[i+1].name == NULL))
				continue;
		// get data for a line
		astro::RaDec	from;
		from.ra().hours(constellation_points[i].ra);
		from.dec().degrees(constellation_points[i].dec);
		astro::RaDec	to;
		to.ra().hours(constellation_points[i+1].ra);
		to.dec().degrees(constellation_points[i+1].dec);
		// draw the line
		drawLine(painter, from, to);
	}
}

} // namespace snowgui
