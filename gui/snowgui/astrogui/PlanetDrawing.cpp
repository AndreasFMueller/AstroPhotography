/*
 * PlanetDrawing.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <PlanetDrawing.h>
#include <AstroSolarsystem.h>
#include <AstroFormat.h>
#include <time.h>
#include <sstream>
#include <QPainterPath>

using namespace astro::catalog;

namespace snowgui {

/**
 * \brief Construct a PlanetDrawing widget
 */
PlanetDrawing::PlanetDrawing() {
	_radius = 10;
}

/**
 * \brief Destroy the sky drawing widget
 */
PlanetDrawing::~PlanetDrawing() {
}

/**
 * \brief Draw a solar system body as a circle
 *
 * \param painter	Pointer to use for drawing
 * \param position	position of the object
 * \param radius	radius of the circle to draw
 * \param color		color of the object
 */
void	PlanetDrawing::drawSolarsystemBody(QPainter& painter,
		const astro::RaDec& pos,
		double radius, QColor color, QString label) {
	QPointF	center;
	try {
		center = this->position(pos);
	} catch (const std::exception& x) {
		std::string	msg = astro::stringprintf("cannot draw %s: %s",
			label.unicode(), x.what());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		return;
	}

	// draw the body
	QPainterPath	circle;
	circle.addEllipse(center, radius, radius);
	painter.fillPath(circle, color);

	// draw the label
	if (label.length() == 0) {
		return;
	}
	QPen	pen(Qt::SolidLine);
	pen.setColor(color);
	painter.setPen(pen);
	painter.drawText(center.x() - 15, center.y() - 20,
		30, 20, Qt::AlignCenter, label);
}

/**
 * \brief Draw the moon
 *
 * \param painter	QPainter to draw on
 */
void	PlanetDrawing::drawMoon(QPainter& painter, time_t t) {
	astro::solarsystem::Moon	moon;
	astro::RaDec	moonposition = moon.ephemeris(displaytime(t));

	// prepare for drawing
	QColor	moonblue(0,204,255);
	double	mr = _radius;
	if (mr < 7) { mr = 7.; }

	drawSolarsystemBody(painter, moonposition, mr, moonblue, QString());
}

/**
 * \brief Draw the sun
 *
 * \param painter	QPainter to draw on
 */
void	PlanetDrawing::drawSun(QPainter& painter, time_t t) {
	astro::solarsystem::Sun	sun;
	astro::RaDec	sunposition = sun.ephemeris(displaytime(t));

	// prepare for drawing
	QColor	sunyellow(255,255,0);
	double	sr = _radius;
	if (sr < 7) { sr = 7.; }

	drawSolarsystemBody(painter, sunposition, sr, sunyellow, QString());
}

/**
 * \brief Draw a planet
 *
 * \param painter		the painter to draw on
 * \param rp			relative position for RA/DEC computation
 * \param planet		a pointer to the planet implementation
 * \param pr			planet radius
 * \param planetcolor		color of the planet
 * \param label			the label to place next to the planet
 */
void	PlanetDrawing::drawPlanet(QPainter& painter,
		astro::solarsystem::RelativePosition& rp,
		astro::solarsystem::Planetoid *planet,
		double pr, QColor planetcolor, QString label) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "drawing %s", planet->name().c_str());
	astro::RaDec	planetposition = rp.radec(planet);
	drawSolarsystemBody(painter, planetposition, pr, planetcolor, label);
	delete planet;
}

/**
 * \brief Draw the sun
 *
 * \param painter	QPainter to draw on
 */
void	PlanetDrawing::drawPlanets(QPainter& painter, time_t t) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "drawing planets for time %d", t);
	astro::solarsystem::JulianCenturies	T(displaytime(t));
	astro::solarsystem::Earth	earth;
	astro::solarsystem::EclipticalCoordinates	earthpos
		= earth.ecliptical(T);
	astro::solarsystem::RelativePosition	rp(T, earthpos);

	// determine planetary radius
	double	pr = 0.5 * _radius;
	if (pr < 4) { pr = 4.; }

	astro::solarsystem::Planetoid	*mercury = new astro::solarsystem::Mercury();
	drawPlanet(painter, rp, mercury, 4, QColor(255,255,204), QString("☿"));

	astro::solarsystem::Planetoid	*venus = new astro::solarsystem::Venus();
	drawPlanet(painter, rp, venus, 4, QColor(255,255,204), QString("♀︎"));

	astro::solarsystem::Planetoid	*mars = new astro::solarsystem::Mars();
	drawPlanet(painter, rp, mars, 4, QColor(255,51,51), QString("♂︎"));

	astro::solarsystem::Planetoid	*jupiter = new astro::solarsystem::Jupiter();
	drawPlanet(painter, rp, jupiter, 4, QColor(255,255,204), QString("♃"));

	astro::solarsystem::Planetoid	*saturn = new astro::solarsystem::Saturn();
	drawPlanet(painter, rp, saturn, 4, QColor(255,153,153), QString("♄"));

	astro::solarsystem::Planetoid	*uranus = new astro::solarsystem::Uranus();
	drawPlanet(painter, rp, uranus, 4, QColor(0,204,102), QString("⛢"));

	astro::solarsystem::Planetoid	*neptune = new astro::solarsystem::Neptune();
	drawPlanet(painter, rp, neptune, 4, QColor(51,153,255), QString("♆"));

	astro::solarsystem::Planetoid	*pluto = new astro::solarsystem::Pluto();
	drawPlanet(painter, rp, pluto, 4, QColor(102,0,0), QString("♇"));
}

time_t	PlanetDrawing::displaytime(time_t t) const {
	if (t) {
		return t;
	}
	time(&t);
	return t;
}


} // namespace snowgui
