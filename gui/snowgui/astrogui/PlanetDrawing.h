/*
 * PlanetDrawing.h -- draw the planets
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _PlanetDrawing_h
#define _PlanetDrawing_h

#include <QObject>
#include <QPainter>
#include <QDial>
#include <AstroUtils.h>
#include <AstroTypes.h>
#include <AstroCatalog.h>
#include <AstroCoordinates.h>
#include <AstroHorizon.h>
#include <AstroSolarsystem.h>
#include <set>
#include <cmath>
#include <deque>

namespace snowgui {

class PlanetData {
public:
	astro::RaDec				position;
	astro::solarsystem::PlanetoidPtr	planetoid;
	double					radius;
	QColor					color;
	QString					label;
};
typedef std::shared_ptr<PlanetData>	PlanetDataPtr;

/**
 * \brief Class that does the drawing of a sky in the SkyView
 */
class PlanetDrawing {
	Q_GADGET
	time_t	displaytime(time_t t) const;
	double	_radius;
	time_t	_cache_time;
	std::list<PlanetDataPtr>	_cache;
	time_t	_sun_time;
	astro::RaDec	_sun_position;
	time_t	_moon_time;
	astro::RaDec	_moon_position;
public:
	explicit	PlanetDrawing();
	virtual ~PlanetDrawing();
	double	radius() const { return _radius; }
	void	radius(double r) { _radius = r; }
	// must override this method to compute the position in the drawing
	// the method is expected to throw an exception if the object is outside
 	// the widget
	virtual	QPointF	position(const astro::RaDec& pos) = 0;
private:
	void	drawSolarsystemBody(QPainter& painter,
			const astro::RaDec& position,
			double radius, QColor color, QString label,
			const std::string& logname);
	void    drawPlanet(QPainter& painter, const astro::RaDec& position,
			astro::solarsystem::PlanetoidPtr planet, double pr,
			QColor planetcolor, QString label);
	void	drawPlanet(QPainter& painter, PlanetDataPtr planet);
	void	drawCachedPlanets(QPainter& painter);
	PlanetDataPtr	makePlanet(astro::solarsystem::RelativePosition& rp,
		astro::solarsystem::PlanetoidPtr planetoid, double radius,
		QColor color, QString label);
public:
	void	drawSun(QPainter& painter, time_t t = 0);
	void	drawMoon(QPainter& painter, time_t t = 0);
	void	drawPlanets(QPainter& painter, time_t t = 0);
};

} // namespace snowgui

#endif /* _PlanetDrawing_h */
