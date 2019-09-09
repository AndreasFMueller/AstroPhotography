/*
 * SkyDrawing.h -- draw the sky
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _SkyDrawing_h
#define _SkyDrawing_h

#include <QObject>
#include <QPainter>
#include <AstroUtils.h>
#include <AstroTypes.h>
#include <AstroCatalog.h>
#include <AstroCoordinates.h>
#include <set>
#include <cmath>
#include <deque>

namespace snowgui {

/**
 * \brief Class representing a point on the Sky View
 */
class SkyPoint {
	bool	_interior;
	astro::Point	_point;
public:
	SkyPoint(float x, float y, bool normalize = true);
	SkyPoint(const astro::Point& point, bool normalize = true);
	SkyPoint(const astro::AzmAlt& azmalt, bool normalize = true);
	bool	interior() const { return _interior; }
	bool	boundary() const { return !_interior; }
	void	interior(bool i) { _interior = i; }
	const astro::Point&	point() const { return _point; }
	void	point(const QPointF& p) { _point = astro::Point(p.x(), p.y()); }
	QPointF	qpoint(float radius, const QPointF& center) const;
	float	phi() const;
};

/**
 * \brief class representing a path on the Sky View
 */
class SkyPath : public std::list<SkyPoint> {
	bool	_hasInteriorPoints;
public:
	SkyPath(const astro::catalog::OutlinePtr outline,
		astro::AzmAltConverter& _converter);
	bool	hasInteriorPoints() const { return _hasInteriorPoints; }
};

/**
 * \brief Class that does the drawing of a sky in the SkyView
 */
class SkyDrawing {
	Q_GADGET
	astro::catalog::Catalog::starsetptr	_stars;
protected:
	astro::AzmAltConverter	*_converter;
private:
	bool	_show_altaz;
	bool	_show_radec;
	bool	_show_pole;
	bool	_show_ecliptic;
	bool	_show_constellations;
	bool	_show_constellation_labels;
	bool	_show_telescope;
	bool	_show_telescope_coord;
	bool	_show_target;
	bool	_show_target_coord;
	bool	_show_labels;
	bool	_show_milkyway;
	bool	_show_position;
	bool	_show_copyright;
	bool	_show_time;
	astro::RaDec	_telescope;
	astro::RaDec	_target;
	astro::LongLat	_position;
	time_t		_time;
	QSize	_size;
public:
	explicit	SkyDrawing();
	virtual ~SkyDrawing();
	bool	show_altaz() const { return _show_altaz; }
	void	show_altaz(bool a) { _show_altaz = a; }
	bool	show_radec() const { return _show_radec; }
	void	show_radec(bool r) { _show_radec = r; }
	bool	show_pole() const { return _show_pole; }
	void	show_pole(bool p) { _show_pole = p; }
	bool	show_ecliptic() const { return _show_ecliptic; }
	void	show_ecliptic(bool r) { _show_ecliptic = r; }
	bool	show_constellations() const { return _show_constellations; }
	void	show_constellations(bool c) { _show_constellations = c; }
	bool	show_constellation_labels() const { return _show_constellation_labels; }
	void	show_constellation_labels(bool l) { _show_constellation_labels = l; }
	bool	show_telescope() const { return _show_telescope; }
	void	show_telescope(bool c) { _show_telescope = c; }
	bool	show_telescope_coord() const { return _show_telescope_coord; }
	void	show_telescope_coord(bool c) { _show_telescope_coord = c; }
	bool	show_target() const { return _show_target; }
	void	show_target(bool c) { _show_target = c; }
	bool	show_target_coord() const { return _show_target_coord; }
	void	show_target_coord(bool c) { _show_target_coord = c; }
	bool	show_labels() const { return _show_labels; }
	void	show_labels(bool l) { _show_labels = l; }
	bool	show_milkyway() const { return _show_milkyway; }
	void	show_milkyway(bool s) { _show_milkyway = s; }
	bool	show_position() const { return _show_position; }
	void	show_position(bool p) { _show_position = p; }
	bool	show_copyright() const { return _show_copyright; }
	void	show_copyright(bool c) { _show_copyright = c; }
	bool	show_time() const { return _show_time; }
	void	show_time(bool t) { _show_time = t; }
	void	telescope(const astro::RaDec& t) { _telescope = t; }
	const astro::RaDec&	telescope() const { return _telescope; }
	void	target(const astro::RaDec& t) { _target = t; }
	const astro::RaDec&	target() const { return _target; }
	void	position(const astro::LongLat& p) { _position = p; }
        const astro::LongLat&	position() const { return _position; }
	void	time(time_t t) { _time = t; }
	time_t	time() const { return _time; }

public:
	virtual void	telescopeChanged(astro::RaDec);
	virtual void	targetChanged(astro::RaDec);
	virtual void	positionChanged(astro::LongLat);
	virtual void	useStars(astro::catalog::Catalog::starsetptr);

protected:
	float	_radius;
	QPointF	_center;
private:
	void	drawLine(QPainter& painter, const astro::RaDec& from,
			const astro::RaDec& to);
	void	drawStar(QPainter& painter, const astro::catalog::Star& star);
	void	drawTelescope(QPainter& painter);
	void	drawTelescopeCoord(QPainter& painter);
	void	drawAltaz(QPainter& painter);
	void	drawRadec(QPainter& painter);
	void	drawPole(QPainter& painter);
	void	drawEcliptic(QPainter& painter);
	void	drawTarget(QPainter& painter);
	void	drawTargetCoord(QPainter& painter);
	void	drawConstellations(QPainter& painter);
	void	drawConstellationLabels(QPainter& painter);
	void	drawPosition(QPainter& painter);
	void	drawCopyright(QPainter& painter);
	void	drawTime(QPainter& painter);
	void	drawLabels(QPainter& painter);
	void	drawMilkyWayOutline(QPainter& painter,
			astro::catalog::OutlinePtr outline,
			astro::catalog::MilkyWay::level_t level,
			QBrush& brush);
	void	drawMilkyWayLevel(QPainter& painter,
			astro::catalog::MilkyWayPtr milkyway,
			astro::catalog::MilkyWay::level_t level);
	void	drawMilkyWay(QPainter& painter);
protected:
	virtual void	redraw();
	astro::AzmAlt	convert(const astro::RaDec& radec);
	QPointF	convert(const astro::AzmAlt& azmalt);
	std::pair<bool, QPointF>	convertlimited(const astro::RaDec& radec);
public:
	void	draw(QPainter& painter, QSize& size);
};

} // namespace snowgui

#endif /* _SkyDrawing_h */
