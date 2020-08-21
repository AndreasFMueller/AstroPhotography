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
	_cache_time = 0;
	_moon_time = 0;
	_sun_time = 0;
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
		double radius, QColor color, QString label,
		const std::string& logname) {
	QPointF	center;
	try {
		center = this->position(pos);
	} catch (const std::exception& x) {
		std::string	msg = astro::stringprintf(
			"cannot draw %s at %s: %s", logname.c_str(),
			pos.toString().c_str(), x.what());
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
	if (_moon_time != displaytime(t)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "compute moon position");
		_moon_time = displaytime(t);
		astro::solarsystem::Moon	moon;
		_moon_position = moon.ephemeris(_moon_time);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "moon at %s",
			_moon_position.toString().c_str());
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "draw cached moon");
	};

	// prepare for drawing
	QColor	moonblue(0,204,255);
	double	mr = _radius;
	if (mr < 7) { mr = 7.; }

	drawSolarsystemBody(painter, _moon_position, mr, moonblue, QString(),
		std::string("moon"));
}

/**
 * \brief Draw the sun
 *
 * \param painter	QPainter to draw on
 */
void	PlanetDrawing::drawSun(QPainter& painter, time_t t) {
	if (_sun_time != displaytime(t)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "compute sun position");
		_sun_time = displaytime(t);
		astro::solarsystem::Sun	sun;
		_sun_position = sun.ephemeris(_sun_time);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "sun at %s",
			_sun_position.toString().c_str());
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "draw cached sun");
	}

	// prepare for drawing
	QColor	sunyellow(255,255,0);
	double	sr = _radius;
	if (sr < 7) { sr = 7.; }

	drawSolarsystemBody(painter, _sun_position, sr, sunyellow, QString(),
		std::string("sun"));
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
		const astro::RaDec& planetposition,
		astro::solarsystem::PlanetoidPtr planet,
		double pr, QColor planetcolor, QString label) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "drawing %s", planet->name().c_str());
	drawSolarsystemBody(painter, planetposition, pr, planetcolor, label,
		planet->name());
}

void	PlanetDrawing::drawPlanet(QPainter& painter, PlanetDataPtr planet) {
	drawPlanet(painter, planet->position, planet->planetoid,
		planet->radius, planet->color, planet->label);
}

void	PlanetDrawing::drawCachedPlanets(QPainter& painter) {
	for (auto p = _cache.begin(); p != _cache.end(); p++) {
		drawPlanet(painter, *p);
	}
}

PlanetDataPtr	PlanetDrawing::makePlanet(
	astro::solarsystem::RelativePosition& rp,
	astro::solarsystem::PlanetoidPtr planetoid,
	double radius, QColor color, QString label) {
	PlanetData	*data = new PlanetData();
	data->position = rp.radec(&*planetoid);
	data->planetoid = planetoid;
	data->radius = radius;
	data->color = color;
	data->label = label;
	return PlanetDataPtr(data);
}

/**
 * \brief Draw the sun
 *
 * \param painter	QPainter to draw on
 */
void	PlanetDrawing::drawPlanets(QPainter& painter, time_t t) {
	time_t	dt = displaytime(t);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "drawing planets for time %d", dt);

	// check whether we can draw cached positions
	if ((dt == _cache_time) || (_cache.size() != 0)) {
		// draw planets from the cache
		drawCachedPlanets(painter);
		return;
	}
	_cache.clear();
	_cache_time = dt;

	astro::solarsystem::JulianCenturies	T(displaytime(t));
	astro::solarsystem::Earth	earth;
	astro::solarsystem::EclipticalCoordinates	earthpos
		= earth.ecliptical(T);
	astro::solarsystem::RelativePosition	rp(T, earthpos);

	// determine planetary radius
	double	pr = 0.5 * _radius;
	if (pr < 4) { pr = 4.; }

	astro::solarsystem::PlanetoidPtr	mercury(
		new astro::solarsystem::Mercury());
	_cache.push_back(makePlanet(rp, mercury, 4, QColor(255,255,204),
		QString("☿")));

	astro::solarsystem::PlanetoidPtr	venus(
		new astro::solarsystem::Venus());
	_cache.push_back(makePlanet(rp, venus, 4, QColor(255,255,204),
		QString("♀︎")));

	astro::solarsystem::PlanetoidPtr	mars(
		new astro::solarsystem::Mars());
	_cache.push_back(makePlanet(rp, mars, 4, QColor(255,51,51),
		QString("♂︎")));

	astro::solarsystem::PlanetoidPtr	jupiter(
		new astro::solarsystem::Jupiter());
	_cache.push_back(makePlanet(rp, jupiter, 4, QColor(255,255,204),
		QString("♃")));

	astro::solarsystem::PlanetoidPtr	saturn(
		new astro::solarsystem::Saturn());
	_cache.push_back(makePlanet(rp, saturn, 4, QColor(255,153,153),
		QString("♄")));

	astro::solarsystem::PlanetoidPtr	uranus(
		new astro::solarsystem::Uranus());
	_cache.push_back(makePlanet(rp, uranus, 4, QColor(0,204,102),
		QString("⛢")));

	astro::solarsystem::PlanetoidPtr	neptune(
		new astro::solarsystem::Neptune());
	_cache.push_back(makePlanet(rp, neptune, 4, QColor(51,153,255),
		QString("♆")));

	astro::solarsystem::PlanetoidPtr	pluto(
		new astro::solarsystem::Pluto());
	_cache.push_back(makePlanet(rp, pluto, 4, QColor(102,0,0),
		QString("♇")));
}

time_t	PlanetDrawing::displaytime(time_t t) const {
	if (t == 0) {
		time(&t);
	}
	t -= t % 60; // round to minutes
	return t;
}


} // namespace snowgui
