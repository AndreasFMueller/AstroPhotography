/*
 * SkyDrawing.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <SkyDrawing.h>
#include <AstroSolarsystem.h>
#include <time.h>
#include <sstream>
#include <QPainterPath>

using namespace astro::catalog;

namespace snowgui {

/**
 * \brief Construct a SkyPoint
 */
SkyPoint::SkyPoint(const astro::AzmAlt& azmalt, bool normalize) {
	float	r = 1 - azmalt.alt().radians() / (M_PI / 2);
	_interior = (r < 1);
	if ((normalize) && (r > 1)) {
		r = 1;
	}
	float	phi = azmalt.azm().radians();
	_point = astro::Point(r * sin(phi), r * cos(phi));
}

SkyPoint::SkyPoint(float x, float y, bool normalize) : _point(x, y) {
	_interior = (hypot(x, y) < 1);
	if ((normalize) && (!_interior)) {
		_point.normalize();
	}
}

SkyPoint::SkyPoint(const astro::Point& point, bool normalize) : _point(point) {
	_interior = (hypot(point.x(), point.y()) < 1);
	if ((normalize) && (!_interior)) {
		_point.normalize();
	}
}

QPointF	SkyPoint::qpoint(float radius, const QPointF& center) const {
	return QPointF(center.x() + radius * _point.x(),
		center.y() + radius * _point.y());
}

float	SkyPoint::phi() const {
	return atan2(-_point.y(), _point.x());
}

SkyPath::SkyPath(const astro::catalog::OutlinePtr outline,
	const astro::AzmAltConverter& converter,
	const SkyRotate& _rotate) {
	_hasInteriorPoints = false;
	for (auto i = outline->begin(); i != outline->end(); i++) {
		SkyPoint	p(converter(*i));
		push_back(_rotate(p));
		_hasInteriorPoints |= p.interior();
	}
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "has%s interior points",
	//	(_hasInteriorPoints) ? "" : " no");
	// make sure the path starts with an interior point
	if (_hasInteriorPoints) {
		while (!front().interior()) {
			push_back(front());
			pop_front();
		}
	}
}

static bool	visible(const astro::AzmAlt& a) {
	return (a.alt().radians() > 0);
}

/**
 * \brief Construct a SkyDrawing widget
 */
SkyDrawing::SkyDrawing() {
	_show_altaz = true;
	_show_radec = true;
	_show_pole = false;
	_show_ecliptic = true;
	_show_constellations = true;
	_show_constellation_labels = true;
	_show_telescope = false;
	_show_target = false;
	_show_telescope_coord = false;
	_show_target_coord = false;
	_show_labels = false;
	_show_milkyway = true;
	_show_position = false;
	_show_time = false;
	_show_copyright = false;
	_show_horizon = false;
	_show_sun = true;
	_show_moon = true;
	_show_planets = true;
	_converter = NULL;
	_time = 0;
	_timeoffset = 0;
}

/**
 * \brief Destroy the sky drawing widget
 */
SkyDrawing::~SkyDrawing() {
	if (_converter) {
		delete _converter;
		_converter = NULL;
	}
}

void	SkyDrawing::redraw() {
}

time_t	SkyDrawing::displaytime() const {
	time_t	t = _time;
	if (!_time) {
		::time(&t);
	}
	t += timeoffset();
	return t;
}

/**
 * \brief convert the celestial coordinates to altitude and azimut
 *
 * \param radec		the celestial coordinates to convert
 */
astro::AzmAlt	SkyDrawing::convert(const astro::RaDec& radec) {
	if (NULL == _converter) {
		_converter = new astro::AzmAltConverter(_position);
	}
	return (*_converter)(radec);
}

/**
 * \brief convert Azimuth and Altitude to pixel coordinates
 *
 * \param azmalt	azimuth and altitude to convert
 */
QPointF	SkyDrawing::convert(const astro::AzmAlt& azmalt) {
	float	r = _radius * (1 - azmalt.alt().radians() / (M_PI / 2));
	double	phi = azmalt.azm().radians();
	QPointF	starcenter(center().x() + r * sin(phi),
			center().y() + r * cos(phi));
	return _rotate(starcenter);
}

/**
 * \brief convert celestial coordinates to a point
 *
 * This method makes sure points outside the circle are mapped to points
 * on the circle
 */
std::pair<bool, QPointF>	SkyDrawing::convertlimited(const astro::RaDec& radec) {
	astro::AzmAlt	azmalt = convert(radec);
	float	r = 1 - azmalt.alt().radians() / (M_PI / 2);
	bool	inside = true;
	if (r > 1) {
		inside = false;
		r = 1;
	}
	r *= _radius;
	double	phi = azmalt.azm().radians();
	QPointF	starcenter(center().x() + r * sin(phi),
			center().y() + r * cos(phi));
	return std::make_pair(inside, _rotate(starcenter));
}

/**
 * \brief Draw a line
 *
 * This method takes care that lines that have both alts negative are not
 * drawn at all, and lines that have precisely one alt negative are 
 * interpolated in a way so that they can be drawn precisely to the boundary.
 *
 * \param painter	painter to use for drawing
 * \param from		initial point
 * \param to		target point
 */
void	SkyDrawing::drawLine(QPainter& painter, const astro::RaDec& from,
		const astro::RaDec& to) {
	astro::AzmAlt	From = convert(from);
	astro::AzmAlt	To = convert(to);

	// segment completely outside the circle
	if ((From.alt().radians() < 0) && (To.alt().radians() < 0)) {
		return;
	}

	// segment completely inside the circle
	if ((From.alt().radians() > 0) && (To.alt().radians() > 0)) {
		QPointF	F = convert(From);
		QPointF	T = convert(To);
		painter.drawLine(F, T);
		return;
	}

	// remaining cases, start with the differences
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "divide %.3f/%.3f - %.3f/%.3f",
	//	From.azm().degrees(), From.alt().degrees(),
	//	To.azm().degrees(), From.alt().degrees());
	astro::Angle	delta = To.azm() - From.azm();
	if (delta.radians() > M_PI) {
		delta.radians(delta.radians() - 2 * M_PI);
	}
	if (delta.radians() < -M_PI) {
		delta.radians(delta.radians() + 2 * M_PI);
	}
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "divide delta: %.4f degrees",
	//	delta.degrees());

	// now divide the segment to get the boundary point
	double	t = From.alt().radians() / (To.alt() - From.alt()).radians();
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "divide with t = %.3f", t);
	astro::AzmAlt	boundary;
	boundary.azm() = From.azm() - delta * t;

	// now draw the segment to the boundary
	QPointF	B = convert(boundary);
	if (From.alt().radians() > 0) {
		QPointF	F = convert(From);
		painter.drawLine(F, B);
	} else {
		QPointF	T = convert(To);
		painter.drawLine(B, T);
	}
}

/**
 * \brief Draw a star
 *
 * \param painter	the QPainter to use to draw the star
 * \param star		the star to draw
 */
void	SkyDrawing::drawStar(QPainter& painter, const Star& star) {
	// find azimuth and altitude
	astro::AzmAlt	azmalt = convert(star.position(2000));

	// decide whether to draw the star at all
	if (!visible(azmalt)) {
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "skipping star %s",
		//	star.toString().c_str());
		return;
	}

	// compute coordinates where to draw the star
	QPointF	starcenter = convert(azmalt);

	// compute the radius of the circle from the magnitude of the star
	float	sr = 4 - star.mag() / 1.8;
	if (sr < 0.8) {
		sr = 0.8;
	}
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "drawing star %s at %s r=%.1f",
	//	star.toString().c_str(), S(azmalt).c_str(), sr);

	// now prepare a path for the star
	QPainterPath	starcircle;
	starcircle.addEllipse(starcenter, sr, sr);

	// draw the star
	QColor	white(255, 255, 255);
	painter.fillPath(starcircle, white);
}

/**
 * \brief Draw a telescope marker
 *
 * \param painter	the QPainter to use to draw the telescope marker
 */
void	SkyDrawing::drawTelescope(QPainter& painter) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw telescope marker");
	// find out where to draw the marker
	astro::AzmAlt	azmalt = convert(telescope());
	if (!visible(azmalt)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "telescope below horizon");
		return;
	}

	// set up drawing the telescope marker
	QPainterPath	telescopemarker;
	QPen	pen(Qt::SolidLine);
	pen.setWidth(2);
	QColor	red(255, 0, 0);
	pen.setColor(red);
	painter.setPen(pen);

	// compose the path
	QPointF	markerpoint = convert(azmalt);
	telescopemarker.addEllipse(markerpoint, 7, 7);
	telescopemarker.moveTo(QPointF(markerpoint.x(), markerpoint.y() - 12));
	telescopemarker.lineTo(QPointF(markerpoint.x(), markerpoint.y() + 12));
	telescopemarker.moveTo(QPointF(markerpoint.x() - 12, markerpoint.y()));
	telescopemarker.lineTo(QPointF(markerpoint.x() + 12, markerpoint.y()));

	// draw the marker in red
	painter.drawPath(telescopemarker);
}

/**
 * \brief Draw a telescope marker
 *
 * \param painter	the QPainter to use to draw the telescope marker
 */
void	SkyDrawing::drawTelescopeCoord(QPainter& painter) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "print telescope position");
	QPen	pen(Qt::SolidLine);
	painter.setPen(pen);
	QColor	red(255, 0, 0);
	pen.setColor(red);
	painter.setPen(pen);
	painter.drawText(center().x(), 0, center().x(), 20,
		Qt::AlignRight, QString(telescope().toString().c_str()));
}

/**
 * \brief Draw a target marker
 *
 * \param painter	the QPainter to use to draw the telescope marker
 */
void	SkyDrawing::drawTarget(QPainter& painter) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw target marker");
	// find out where to draw the marker
	astro::AzmAlt	t = convert(target());
	if (!visible(t)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "target below horizon");
		return;
	}

	// set up drawing the telescope marker
	QPainterPath	targetmarker;
	QPen	pen(Qt::SolidLine);
	pen.setWidth(2);
	QColor	green(0, 255, 0);
	pen.setColor(green);
	painter.setPen(pen);

	// compose the path
	QPointF	markerpoint = convert(t);
	targetmarker.addEllipse(markerpoint, 7, 7);

	// draw the marker in red
	painter.drawPath(targetmarker);
}

/**
 * \brief Draw the darget coordinates
 *
 * \param painter	the QPainter to use to draw the telescope marker
 */
void	SkyDrawing::drawTargetCoord(QPainter& painter) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "print target coordinates");
	QPen	pen(Qt::SolidLine);
	QColor	green(0, 255, 0);
	pen.setColor(green);
	painter.setPen(pen);
	painter.drawText(center().x(), 20, center().x(), 20,
		Qt::AlignRight, QString(target().toString().c_str()));
}

/**
 * \brief Draw the AltAz grid
 *
 * \param painter	the QPainter to use to draw the telescope marker
 */
void	SkyDrawing::drawAltaz(QPainter& painter) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw AltAz grid");
	// prepare a pen for drawing
	QPen	pen(Qt::SolidLine);
	pen.setWidth(1);
	QColor	orange(255,204,0);
	pen.setColor(orange);
	painter.setPen(pen);

	// draw the circles
	for (double r = 1./3; r < 1.1; r += 1./3) {
		QPainterPath	path;
		path.addEllipse(center(), r * _radius, r * _radius);
		painter.drawPath(path);
	}
	
	// draw the radial lines
	for (double a = 0; a < M_PI; a += M_PI / 6) {
		QPointF	p1(center().x() + _radius * cos(a),
				center().y() + _radius * sin(a));
		QPointF	p2(center().x() - _radius * cos(a),
				center().y() - _radius * sin(a));
		painter.drawLine(_rotate(p1), _rotate(p2));
	}
}

/**
 * \brief Draw the RA/DEC grid
 *
 * \param painter	the QPainter to use to draw the telescope marker
 */
void	SkyDrawing::drawRadec(QPainter& painter) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw RaDec grid");
	// prepare a pen for drawing
	QPen	pen(Qt::SolidLine);
	pen.setWidth(1);
	QColor	blue(102,204,255);
	pen.setColor(blue);
	painter.setPen(pen);

	// draw constant RA lines
	double	l = M_PI - 0.0001;
	double	decstep = l / 100;
	double	declimit = l/2 - decstep/2;
	for (double ra = 0; ra < 2 * M_PI; ra += M_PI / 6) {
		for (double dec = -l / 2; dec < declimit; dec += decstep) {
			astro::RaDec	from(ra, dec), to(ra, dec + decstep);
			drawLine(painter, from, to);
		}
	}

	// draw the DEC lines
	double	rastep = M_PI / 100;
	double	ralimit = 2 * M_PI - rastep/2;
	for (double dec = M_PI / 2; dec > -M_PI/2; dec -= M_PI / 6) {
		for (double ra = 0; ra < ralimit; ra += rastep) {
			astro::RaDec	from(ra, dec), to(ra + rastep, dec);
			drawLine(painter, from, to);
		}
	}
}

/**
 * \brief Draw the pole
 *
 * \param painter	the QPinter to use fro drawing
 */
void	SkyDrawing::drawPole(QPainter& painter) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw pole");
	// prepare a pen for drawing
	QPen	pen(Qt::SolidLine);
	pen.setWidth(1);
	QColor	blue(102,204,255);
	pen.setColor(blue);
	painter.setPen(pen);

	// handle north and south point
	std::pair<bool, QPointF>	N
		= convertlimited(astro::RaDec::north_pole);
	if (N.first) {
		QPainterPath	painterpath;
		painterpath.addEllipse(N.second.x() - 10, N.second.y() - 10, 20, 20);
		painter.fillPath(painterpath, Qt::black);
		painter.drawText(N.second.x() - 10, N.second.y() - 10,
			20, 20, Qt::AlignCenter, QString("N"));
	}
	std::pair<bool, QPointF>	S
		= convertlimited(astro::RaDec::south_pole);
	if (S.first) {
		QPainterPath	painterpath;
		painterpath.addEllipse(S.second.x() - 10, S.second.y() - 10, 20, 20);
		painter.fillPath(painterpath, Qt::black);
		painter.drawText(S.second.x() - 10, S.second.y() - 10,
			20, 20, Qt::AlignCenter, QString("S"));
	}
}

/**
 * \brief Draw the horizon
 *
 * \param painter	the QPinter to use fro drawing
 */
void	SkyDrawing::drawHorizon(QPainter& painter) {
	if (!_horizon) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no horizon present");
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw the horizon");

	// build a path
	QPainterPath	horizonpath;

	// start point
	astro::AzmAlt	startpoint = *_horizon->begin();
	startpoint = astro::AzmAlt(
		astro::Angle(startpoint.azm() + astro::Angle(M_PI)),
		startpoint.alt());

	// start the path from the 0/0 point
	astro::AzmAlt	basepoint(astro::Angle(M_PI), astro::Angle(0.));
	horizonpath.moveTo(convert(basepoint));

	// first go around the circle along the horizon
	for (auto i = _horizon->begin(); i != _horizon->end(); i++) {
		astro::AzmAlt	a = *i;
		astro::AzmAlt	b(a.azm() + astro::Angle(M_PI), a.alt());
		horizonpath.lineTo(convert(b));
	}
	horizonpath.lineTo(convert(startpoint));
	horizonpath.lineTo(convert(basepoint));

	// go around the outer diameter in the opposite direction
	for (double angle = 359; angle > 0; angle--) {
		horizonpath.lineTo(convert(astro::AzmAlt(
			astro::Angle(angle + 180, astro::Angle::Degrees),
			astro::Angle(0.))));
	}
	horizonpath.closeSubpath();;

	// fill the path
	QColor	terrain(64, 64, 64, 196);
	painter.fillPath(horizonpath, terrain);

	// draw a dark grey line for the horizon
	QPainterPath	horizonline;
	horizonline.moveTo(convert(startpoint));

	// prepare a pen for the horizon line
	QPen	pen(Qt::SolidLine);
	pen.setWidth(2);
	QColor	darkgray(32, 32, 32);
	pen.setColor(darkgray);
	painter.setPen(pen);

	// go around the horizon for the horizon line
	auto	i = _horizon->begin();
	for (i++; i != _horizon->end(); i++) {
		astro::AzmAlt	a = *i;
		astro::AzmAlt	b(a.azm() + astro::Angle(M_PI), a.alt());
		horizonline.lineTo(convert(b));
	}
	horizonline.closeSubpath();;

	// draw the horizon line
	painter.drawPath(horizonline);
}

static astro::RaDec	ecliptic_point(const astro::Angle ra) {
	astro::Angle	dec = astro::arctan(sin(ra) * sin(astro::Angle::ecliptic_angle));
	return astro::RaDec(ra, dec);
}

/**
 * \brief Draw the Ecliptic
 *
 * \param painter	the QPainter to use to draw the telescope marker
 */
void	SkyDrawing::drawEcliptic(QPainter& painter) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw ecliptic");
	// prepare green pen for drawing
	QPen	pen(Qt::SolidLine);
	pen.setWidth(1);
	QColor	green(51,153,51);
	pen.setColor(green);
	painter.setPen(pen);

	// draw the ecliptic
	astro::Angle	step(5 * M_PI / 180);
	for (int i = 0; i <= 355 / 5; i++) {
		astro::Angle	ra = step * i;
		astro::RaDec	from = ecliptic_point(ra);
		ra = step * (i + 1);
		astro::RaDec	to = ecliptic_point(ra);
		drawLine(painter, from, to);
	}
}

/**
 * \brief Draw the position label in the lower left corner
 *
 * \param painter	the QPainter to use to draw the telescope marker
 */
void	SkyDrawing::drawPosition(QPainter& painter) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "print position");
	QPen	pen(Qt::SolidLine);
	pen.setColor(Qt::white);
	painter.setPen(pen);

        astro::Angle   lo = _position.longitude();
        while (lo > astro::Angle(M_PI)) { lo = lo - astro::Angle(M_PI); }
        while (lo < astro::Angle(-M_PI)) { lo = lo + astro::Angle(M_PI); }
        astro::Angle   la = _position.latitude();
        std::ostringstream      out;
        if (lo >= 0) {
                out << "E ";
        } else {
                out << "W ";
        }
        out      << lo.dms(':', 0).substr(1);
        if (la >= 0) {
                out << " N ";
        } else {
                out << " S ";
        }
        out      << la.dms(':', 0).substr(1);

	//painter.drawText(0, _size.height() - 40, _radius, 40,
	//	Qt::AlignLeft, QString(out.str().c_str()));
	painter.drawText(0, 0, _radius, 20,
		Qt::AlignLeft, QString(out.str().c_str()));
}

/**
 * \brief show the time in the lower left corner
 *
 * \param painter	the QPainter to use to draw the telescope marker
 */
void	SkyDrawing::drawTime(QPainter& painter) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "print time");
	time_t	t = displaytime();
	struct tm	*tmp = localtime(&t);
	char	buffer[128];
	strftime(buffer, sizeof(buffer), "%F %T", tmp);
	QPen	pen(Qt::SolidLine);
	pen.setColor(Qt::white);
	painter.setPen(pen);
	//painter.drawText(0, _size.height() - 20, _radius, 20,
	//	Qt::AlignLeft, QString(buffer));
	painter.drawText(0, 20, _radius, 20,
		Qt::AlignLeft, QString(buffer));
}

/**
 * \brief show the copyright in the lower right corner
 *
 * \param painter	the QPainter to use to draw the telescope marker
 */
void	SkyDrawing::drawCopyright(QPainter& painter) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "print copyright");
	QPen	pen(Qt::SolidLine);
	pen.setColor(Qt::white);
	painter.setPen(pen);
	painter.drawText(center().x(), _size.height() - 20, _radius, 20,
		Qt::AlignRight, QString("(c) 2019 AstroPhotography"));
}

/**
 * \brief paint the sky anew
 *
 * \param painter	the QPainter to use to draw the telescope marker
 * \param size		size of the pixmap to draw on
 */
void	SkyDrawing::draw(QPainter& painter, QSize& size) {
	if (NULL != _converter) {
		delete _converter;
	}
	_converter = new astro::AzmAltConverter(_position);
	if (_time) {
		_converter->update(_time + timeoffset());
	} else {
		_converter->update(displaytime()); // updates to the current time
	}

	// set up the parameters of drawing: radius and center
	_size = size;
	_radius = std::min<float>(size.width() / 2., size.height() / 2);
	center(QPointF(size.width() / 2, size.height() / 2));

	// set up a painter for drawing operations
	painter.setRenderHint(QPainter::Antialiasing);

	// draw a black circle
	QPainterPath	circle;
	circle.addEllipse(center(), _radius, _radius);
	QColor	black(0, 0, 0);
	painter.fillPath(circle, black);

	// first draw the milkyway
	if (show_milkyway()) {
		try {
			drawMilkyWay(painter);
		} catch (const std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot draw milkyway: %s",
				x.what());
		}
	}

	// draw the grids
	if (show_altaz()) {
		drawAltaz(painter);
	}
	if (show_radec()) {
		drawRadec(painter);
	}
	if (show_pole()) {
		drawPole(painter);
	}
	if (show_ecliptic()) {
		drawEcliptic(painter);
	}
	if (show_constellations()) {
		drawConstellations(painter);
	}
	if (show_constellation_labels()) {
		drawConstellationLabels(painter);
	}

	// draw the stars
	{
		std::lock_guard<std::recursive_mutex>	lock(_mutex);
		if (_stars) {
			Catalog::starset::const_iterator	i;
			for (i = _stars->begin(); i != _stars->end(); i++) {
				drawStar(painter, *i);
			}
		}
	}

	// draw solar system objects
	if (show_sun()) {
		drawSun(painter);
	}
	if (show_moon()) {
		drawMoon(painter);
	}
	if (show_planets()) {
		drawPlanets(painter);
	}

	// draw the horizon
	if (show_horizon()) {
		drawHorizon(painter);
	}

	// draw the telescope marker
	if (show_telescope()) {
		drawTelescope(painter);
	}
	if (show_telescope_coord()) {
		drawTelescopeCoord(painter);
	}
	if (show_target()) {
		drawTarget(painter);
	}
	if (show_target_coord()) {
		drawTargetCoord(painter);
	}

	if (show_labels()) {
		drawLabels(painter);
	}

	if (show_copyright()) {
		drawCopyright(painter);
	}
	if (show_position()) {
		drawPosition(painter);
	}
	if (show_time()) {
		drawTime(painter);
	}
}

/**
 * \brief Draw direction labels
 *
 * \param painter	painter to draw the milkyway with
 */
void	SkyDrawing::drawLabels(QPainter& painter) {
	QPen	pen;
	pen.setColor(Qt::green);
	painter.setPen(pen);

	QPointF	Spoint(center().x(), center().y() + _radius - 10);
	Spoint = _rotate(Spoint);
	painter.drawText(Spoint.x() - 10, Spoint.y() - 10,
		20, 20, Qt::AlignCenter, QString("S"));

	QPointF	Npoint(center().x(), center().y() - _radius + 10);
	Npoint = _rotate(Npoint);
	painter.drawText(Npoint.x() - 10, Npoint.y() - 10,
		20, 20, Qt::AlignCenter, QString("N"));

	QPointF	Epoint(center().x() - _radius + 10, center().y());
	Epoint = _rotate(Epoint);
	painter.drawText(Epoint.x() - 10, Epoint.y() - 10,
		20, 20, Qt::AlignCenter, QString("E"));

	QPointF	Wpoint(center().x() + _radius - 10, center().y());
	Wpoint = _rotate(Wpoint);
	painter.drawText(Wpoint.x() - 10, Wpoint.y() - 10,
		20, 20, Qt::AlignCenter, QString("W"));
}

/**
 * \brief Redraw the sky with a new postion of the telescope marker
 *
 * \param radec		direction into which the telescope is pointing
 */
void	SkyDrawing::telescopeChanged(astro::RaDec radec) {
	telescope(radec);
	redraw();
}

/**
 * \brief Redraw the sky with a new position of the telescope on earth
 *
 * \param longlat	geographical position of the observatory on earth
 */
void	SkyDrawing::positionChanged(astro::LongLat longlat) {
	position(longlat);
	redraw();
}

/**
 * \brief slot to give stars to the display widget
 */
void	SkyDrawing::useStars(Catalog::starsetptr stars) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got stars");
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	_stars = stars;
	redraw();
}

void	SkyDrawing::targetChanged(astro::RaDec target) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new target: %s",
		target.toString().c_str());
	SkyDrawing::target(target);
	if (show_target()) {
		redraw();
	}
}

/**
 * \brief Draw the Milkyway
 *
 * \param painter	painter to draw the milkyway with
 */
void	SkyDrawing::drawMilkyWay(QPainter& painter) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw milkyway");
	astro::catalog::MilkyWayPtr	milkyway = MilkyWay::get();
	drawMilkyWayLevel(painter, milkyway, astro::catalog::MilkyWay::L1);
	drawMilkyWayLevel(painter, milkyway, astro::catalog::MilkyWay::L2);
	drawMilkyWayLevel(painter, milkyway, astro::catalog::MilkyWay::L3);
	drawMilkyWayLevel(painter, milkyway, astro::catalog::MilkyWay::L4);
	drawMilkyWayLevel(painter, milkyway, astro::catalog::MilkyWay::L5);
}

/**
 * \brief Draw the outlines for a given level
 *
 * There is a small problem with level L1 as the curves cannot individually
 * be filled. This means that we treet level L1 differently and draw only
 * the outline.
 *
 * \param painter	painter to draw the milkyway with
 * \param milkyway	the milkyway structure to take the outlines from
 * \param level		the level to draw
 */
void	SkyDrawing::drawMilkyWayLevel(QPainter& painter,
		astro::catalog::MilkyWayPtr milkyway,
		astro::catalog::MilkyWay::level_t level) {
	int	l = 64 + 16 * (int)level;
	QColor	brushcolor(l, l, l);
	QBrush	brush(brushcolor);
	int	L = l;
	QColor	pencolor(L, L, L);
	QPen	pen;
	pen.setColor(pencolor);
	pen.setWidth(2);
	painter.setPen(pen);
	astro::catalog::OutlineListPtr	outlines = (*milkyway)[level];
	// go through the outlines
	SkyDrawing	*skydrawing = this;
	for_each(outlines->begin(), outlines->end(),
		[&painter,skydrawing,level,&brush](OutlinePtr& outline)
			mutable {
			skydrawing->drawMilkyWayOutline(painter, outline,
				level, brush);
		}
	);
}

/**
 * \brief Draw an outline
 *
 * note that the level is needed to decide whether to fill the outline or
 * not. Only Levels above L1 can be filled.
 *
 * \param painter	painter to draw the milkyway with
 * \param outline	the outline to be drawn
 * \param level		the level of this outline
 * \param brush		the brush to be used for filling
 */
void	SkyDrawing::drawMilkyWayOutline(QPainter& painter,
		astro::catalog::OutlinePtr outline,
		astro::catalog::MilkyWay::level_t level,
		QBrush& brush) {
	// convert the outline into a SkyPath
	SkyPath	path(outline, *_converter, _rotate);
	if (!path.hasInteriorPoints()) {
		//debug(LOG_DEBUG, DEBUG_LOG, 0,
		//	"path has no interior points, skipping");
		return;
	}

	// now draw
	QPainterPath	painterpath;
	auto	i = path.begin();
	SkyPoint	previous = *i;
	painterpath.moveTo(previous.qpoint(_radius, center()));
	int	counter = 0;
	while (++i != path.end()) {
		SkyPoint	next = *i;
		if ((previous.boundary()) && (next.boundary())) {
			float	start = previous.phi() * 180 / M_PI;
			float	end = next.phi() * 180 / M_PI;
			// draw an arc
			float	a = end - start;
			while (a > 180) {
				a -= 360;
			}
			while (a < -180) {
				a += 360;
			}
			painterpath.arcTo(center().x() - _radius,
				center().y() - _radius, 2 * _radius, 2 * _radius,
				start, a);
		} else {
			painterpath.lineTo(next.qpoint(_radius, center()));
		}
		counter++;
		previous = next;
	}
	painterpath.closeSubpath();
	if (level > 0) {
		painter.fillPath(painterpath, brush);
	}
	painter.drawPath(painterpath);
}

/**
 * \brief Draw the constellation labels
 *
 * \param painter	painter to draw the milkyway with
 */
void	SkyDrawing::drawConstellationLabels(QPainter& painter) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw constellation labels");
	// set the constellation color
	QPen	pen(Qt::SolidLine);
	QColor	pink(255,0,204);
	pen.setColor(pink);
	painter.setPen(pen);

	// the the constellation data
	ConstellationCatalogPtr	consts = ConstellationCatalog::get();

	// iterate through the constellations
	for (auto i = consts->begin(); i != consts->end(); i++) {
		std::string	name = i->first;
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "display label for %s",
		//	name.c_str());
		astro::RaDec	pos = i->second->centroid();
		std::pair<bool, QPointF>	p = convertlimited(pos);
		if (p.first) {
			painter.drawText(p.second.x() - 15, p.second.y() - 15,
				30, 20, Qt::AlignCenter, QString(name.c_str()));
		}
	}
}

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

/**
 * \brief Draw a solar system body as a circle
 *
 * \param painter	Pointer to use for drawing
 * \param position	position of the object
 * \param radius	radius of the circle to draw
 * \param color		color of the object
 */
void	SkyDrawing::drawSolarsystemBody(QPainter& painter,
		const astro::RaDec& position,
		double radius, QColor color, QString label) {
	astro::AzmAlt	Position = convert(position);
	if (Position.alt().radians() < 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "object not visible");
		return;
	}

	// draw the body
	QPointF	center = convert(Position);
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
void	SkyDrawing::drawMoon(QPainter& painter) {
	astro::solarsystem::Moon	moon;
	astro::RaDec	moonposition = moon.ephemeris(displaytime());

	// prepare for drawing
	QColor	moonblue(0,204,255);
	double	mr = _radius / 180.;
	if (mr < 7) { mr = 7.; }

	drawSolarsystemBody(painter, moonposition, mr, moonblue, QString());
}

/**
 * \brief Draw the sun
 *
 * \param painter	QPainter to draw on
 */
void	SkyDrawing::drawSun(QPainter& painter) {
	astro::solarsystem::Sun	sun;
	astro::RaDec	sunposition = sun.ephemeris(displaytime());

	// prepare for drawing
	QColor	sunyellow(255,255,0);
	double	sr = _radius / 180.;
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
void	SkyDrawing::drawPlanet(QPainter& painter,
		astro::solarsystem::RelativePosition& rp,
		astro::solarsystem::Planetoid *planet,
		double pr, QColor planetcolor, QString label) {
	astro::RaDec	planetposition = rp.radec(planet);
	drawSolarsystemBody(painter, planetposition, pr, planetcolor, label);
	delete planet;
}

/**
 * \brief Draw the sun
 *
 * \param painter	QPainter to draw on
 */
void	SkyDrawing::drawPlanets(QPainter& painter) {
	astro::solarsystem::JulianCenturies	T(displaytime());
	astro::solarsystem::Earth	earth;
	astro::solarsystem::EclipticalCoordinates	earthpos
		= earth.ecliptical(T);
	astro::solarsystem::RelativePosition	rp(T, earthpos);

	// determine planetary radius
	double	pr = _radius / 180.;
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


} // namespace snowgui
