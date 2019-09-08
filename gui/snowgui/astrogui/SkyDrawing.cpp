/*
 * SkyDrawing.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <SkyDrawing.h>

using namespace astro::catalog;

namespace snowgui {

static bool	visible(const astro::AzmAlt& a) {
	return (a.alt().radians() > 0);
}

/**
 * \brief Construct a SkyDrawing widget
 */
SkyDrawing::SkyDrawing() {
	_show_altaz = true;
	_show_radec = true;
	_show_ecliptic = true;
	_show_constellations = true;
	_show_telescope = false;
	_show_target = false;
	_show_labels = true;
	_show_milkyway = true;
	_converter = NULL;
	_time = 0;
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
	QPointF	starcenter(_center.x() + r * sin(phi),
			_center.y() + r * cos(phi));
	return starcenter;
}

/**
 * \brief convert celestial coordinates to a point
 *
 * This method makes sure points outside the circle are mapped to points
 * on the circle
 */
QPointF	SkyDrawing::convertlimited(const astro::RaDec& radec) {
	astro::AzmAlt	azmalt = convert(radec);
	float	r = 1 - azmalt.alt().radians() / (M_PI / 2);
	if (r > 1) {
		r = 1;
	}
	r *= _radius;
	double	phi = azmalt.azm().radians();
	QPointF	starcenter(_center.x() + r * sin(phi),
			_center.y() + r * cos(phi));
	return starcenter;
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
 * \brief Draw a target marker
 *
 * \param painter	the QPainter to use to draw the telescope marker
 */
void	SkyDrawing::drawTarget(QPainter& painter) {
	// find out where to draw the marker
	astro::AzmAlt	t = convert(_target);
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
 * \brief Draw the AltAz grid
 */
void	SkyDrawing::drawAltaz(QPainter& painter) {
	// prepare a pen for drawing
	QPen	pen(Qt::SolidLine);
	pen.setWidth(1);
	QColor	orange(255,204,0);
	pen.setColor(orange);
	painter.setPen(pen);

	// draw the circles
	for (double r = 1./3; r < 1.1; r += 1./3) {
		QPainterPath	path;
		path.addEllipse(_center, r * _radius, r * _radius);
		painter.drawPath(path);
	}
	
	// draw the radial lines
	for (double a = 0; a < M_PI; a += M_PI / 6) {
		QPointF	p1(_center.x() + _radius * cos(a),
				_center.y() + _radius * sin(a));
		QPointF	p2(_center.x() - _radius * cos(a),
				_center.y() - _radius * sin(a));
		painter.drawLine(p1, p2);
	}
}

/**
 * \brief Draw the RA/DEC grid
 */
void	SkyDrawing::drawRadec(QPainter& painter) {
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

static astro::RaDec	ecliptic_point(const astro::Angle ra) {
	astro::Angle	dec = astro::arctan(sin(ra) * sin(astro::Angle::ecliptic_angle));
	return astro::RaDec(ra, dec);
}

/**
 * \brief Draw the Ecliptic
 */
void	SkyDrawing::drawEcliptic(QPainter& painter) {
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
 * \brief paint the sky anew
 */
void	SkyDrawing::draw(QPainter& painter, QSize& size) {
	if (NULL != _converter) {
		delete _converter;
	}
	_converter = new astro::AzmAltConverter(_position);
	if (_time) {
		_converter->update(_time);
	} else {
		_converter->update(); // updates to the current time
	}

	// set up the parameters of drawing: radius and center
	_radius = std::min<float>(size.width() / 2., size.height() / 2);
	_center = QPointF(size.width() / 2, size.height() / 2);

	// set up a painter for drawing operations
	painter.setRenderHint(QPainter::Antialiasing);

	// draw a black circle
	QPainterPath	circle;
	circle.addEllipse(_center, _radius, _radius);
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
	if (show_ecliptic()) {
		drawEcliptic(painter);
	}
	if (show_constellations()) {
		drawConstellations(painter);
	}

	// draw the stars
	if (_stars) {
		Catalog::starset::const_iterator	i;
		for (i = _stars->begin(); i != _stars->end(); i++) {
			drawStar(painter, *i);
		}
	}

	// draw the telescope marker
	if (show_telescope()) {
		drawTelescope(painter);
	}
	if (show_target()) {
		drawTarget(painter);
	}

	if (show_labels()) {
		drawLabels(painter);
	}
}

/**
 * \brief Draw direction labels
 */
void	SkyDrawing::drawLabels(QPainter& painter) {
	QPen	pen;
	pen.setColor(Qt::green);
	painter.setPen(pen);

	painter.drawText(_center.x() - 10, _center.y() + _radius - 20,
		20, 20, Qt::AlignCenter, QString("S"));

	painter.drawText(_center.x() - 10, _center.y() - _radius,
		20, 20, Qt::AlignCenter, QString("N"));

	painter.drawText(_center.x() - _radius, _center.y() - 10,
		20, 20, Qt::AlignCenter, QString("E"));

	painter.drawText(_center.x() + _radius - 20, _center.y() - 10,
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
	_stars = stars;
	redraw();
}

void	SkyDrawing::targetChanged(astro::RaDec target) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new target: %s",
		target.toString().c_str());
	_target = target;
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw the milkyway");
	astro::catalog::MilkyWayPtr	milkyway = MilkyWay::get();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found the milkyway");
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
	if (L < 128) {
		L = 128;
	}
	QColor	pencolor(L, L, L);
	QPen	pen;
	pen.setColor(pencolor);
	pen.setWidth(1);
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
	QPainterPath	path;
	auto	i = outline->begin();
	path.moveTo(convertlimited(*i));
	int	counter = 0;
	while (++i != outline->end()) {
		path.lineTo(convertlimited(*i));
		counter++;
	}
	path.closeSubpath();
	if (level > 0) {
		painter.fillPath(path, brush);
	}
	painter.drawPath(path);
}

} // namespace snowgui
