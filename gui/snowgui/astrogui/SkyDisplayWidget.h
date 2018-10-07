/*
 * SkyDisplayWidget.h -- Widget to display the visible sky
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _SkyDisplayWidget_h
#define _SkyDisplayWidget_h

#include <QWidget>
#include <QPainter>
#include <set>
#include <cmath>
#include <AstroUtils.h>
#include <AstroCatalog.h>
#include <AstroCoordinates.h>
#include <QThread>

namespace snowgui {

class SkyDisplayWidget;
/**
 * \brief Thread to retrieve set of stars
 */
class SkyStarThread : public QThread {
	Q_OBJECT
	SkyDisplayWidget	*_skydisplaywidget;
public:
	explicit SkyStarThread(QObject *parent = NULL);
	virtual ~SkyStarThread();
	void	run();
signals:
	void	stars(astro::catalog::Catalog::starsetptr);
};

/**
 * \brief Widget to display the sky with stars and telescope marker
 */
class SkyDisplayWidget : public QWidget {
	Q_OBJECT
	QTimer	*_timer;
	astro::catalog::Catalog::starsetptr	_stars;

	astro::AzmAltConverter	*_converter;
	astro::AzmAlt	convert(const astro::RaDec& radec);
	QPointF	_center;
	float	_radius;
	// whether or not to show the altaz grid lines
	bool	_show_altaz;
	bool	_show_radec;
	bool	_show_ecliptic;
	bool	_show_constellations;
	bool	_show_telescope;
	bool	_show_target;
	bool	_show_labels;
	bool	_show_tooltip;
public:
	bool	show_altaz() const { return _show_altaz; }
	void	show_altaz(bool a) { _show_altaz = a; }
	bool	show_radec() const { return _show_radec; }
	void	show_radec(bool r) { _show_radec = r; }
	bool	show_ecliptic() const { return _show_ecliptic; }
	void	show_ecliptic(bool r) { _show_ecliptic = r; }
	bool	show_constellations() const { return _show_constellations; }
	void	show_constellations(bool c) { _show_constellations = c; }
	bool	show_telescope() const { return _show_telescope; }
	void	show_telescope(bool c) { _show_telescope = c; }
	bool	show_target() const { return _show_target; }
	void	show_target(bool c) { _show_target = c; }
	bool	show_labels() const { return _show_labels; }
	void	show_labels(bool l) { _show_labels = l; }
	bool	show_tooltip() const { return _show_tooltip; }
	void	show_tooltip(bool l) { _show_tooltip = l; }

	// telescope direction in right ascension and declination
private:
	astro::RaDec	_telescope;
	astro::RaDec	_target;
public:
	void	telescope(const astro::RaDec& t) { _telescope = t; }
	const astro::RaDec&	telescope() const { return _telescope; }
	void	target(const astro::RaDec& t) { _target = t; }
	const astro::RaDec&	target() const { return _target; }

	// position on earth of the observatory
private:
	astro::LongLat	_position;
public:
	void	position(const astro::LongLat& p) { _position = p; }
	const astro::LongLat&	position() const { return _position; }

	// some private drawing functions
private:
	void	drawLine(QPainter& painter, const astro::RaDec& from,
			const astro::RaDec& to);
	void	drawStar(QPainter& painter, const astro::catalog::Star& star);
	void	drawTelescope(QPainter& painter);
	void	drawAltaz(QPainter& painter);
	void	drawRadec(QPainter& painter);
	void	drawEcliptic(QPainter& painter);
	void	drawTarget(QPainter& painter);
	void	drawConstellations(QPainter& painter);
	void	drawLabels(QPainter& painter);
	void	draw();
	QPointF	convert(const astro::AzmAlt& azmalt);
	astro::RaDec	convert(QMouseEvent *event);

	bool	_mouse_pressed;
protected:
	void	mouseCommon(const astro::RaDec& target);
	void	mousePressEvent(QMouseEvent *e);
	void	mouseReleaseEvent(QMouseEvent *e);
	void	mouseMoveEvent(QMouseEvent *e);
	void	closeEvent(QCloseEvent *e);

	// constructors
public:
	explicit SkyDisplayWidget(QWidget *parent = NULL);
	virtual ~SkyDisplayWidget();

	void	paintEvent(QPaintEvent *event);

public slots:
	void	telescopeChanged(astro::RaDec);
	void	targetChanged(astro::RaDec);
	void	positionChanged(astro::LongLat);
	void	update();
	void	useStars(astro::catalog::Catalog::starsetptr);

signals:
	void	pointSelected(astro::RaDec);
};

} // namespace snowgui 

#endif /* _SkyDisplayWidget_h */
