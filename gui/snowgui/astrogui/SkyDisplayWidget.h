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
#include <AstroHorizon.h>
#include <QThread>
#include <QTimer>
#include <SkyDrawing.h>
#include <OffsetDial.h>
#include <RotateDial.h>

namespace snowgui {

class SkyDisplayWidget;
/**
 * \brief Thread to retrieve set of stars
 */
class SkyStarThread : public QThread {
	Q_OBJECT
	SkyDisplayWidget	*_skydisplaywidget;
	bool	_send_tile;
public:
	bool	send_tile() const { return _send_tile; }
	void	send_tile(bool s) { _send_tile = s; }
	explicit SkyStarThread(QObject *parent = NULL, bool send_tile = false);
	virtual ~SkyStarThread();
	void	run();
signals:
	void	stars(astro::catalog::Catalog::starsetptr);
	void	stars(astro::catalog::StarTilePtr);
};

/**
 * \brief Widget to display the sky with stars and telescope marker
 */
class SkyDisplayWidget : public QWidget, public SkyDrawing {
	Q_OBJECT
	QTimer	_timer;
	bool	_show_tooltip;
	bool	_target_enabled;
	RotateDial	*_rotate_dial;
	OffsetDial	*_timeoffset_dial;
	int	dialsize();
public:
	bool	show_tooltip() const { return _show_tooltip; }
	void	show_tooltip(bool l) { _show_tooltip = l; }

	bool	target_enabled() const { return _target_enabled; }
	void	target_enabled(bool t) { _target_enabled = t; }

	// some private drawing functions
private:
	astro::RaDec	convert(QMouseEvent *event);

	bool	_mouse_pressed;
protected:
	void	mouseCommon(const astro::RaDec& target);
	void	mousePressEvent(QMouseEvent *e);
	void	mouseReleaseEvent(QMouseEvent *e);
	void	mouseMoveEvent(QMouseEvent *e);
	void	closeEvent(QCloseEvent *e);
	void	resizeEvent(QResizeEvent *event);

	// constructors
public:
	explicit SkyDisplayWidget(QWidget *parent = NULL);
	virtual ~SkyDisplayWidget();

	void	paintEvent(QPaintEvent *event);
	virtual void	redraw();

public slots:
	void	telescopeChanged(astro::RaDec);
	void	targetChanged(astro::RaDec);
	void	positionChanged(astro::LongLat);
	void	update();
	void	useStars(astro::catalog::Catalog::starsetptr);
	void	useStars(astro::catalog::StarTilePtr);

	void	setAltAzmGridVisible(bool);
	void	setRaDecGridVisible(bool);
	void	setConstellationsVisible(bool);
	void	setConstellationLabelsVisible(bool);
	void	setTargetVisible(bool);
	void	setTelescopeVisible(bool);
	void	setTooltipVisible(bool);
	void	setLabelsVisible(bool);
	void	setEclipticVisible(bool);
	void	setMilkywayVisible(bool);
	void	setHorizonVisible(bool);
	void	setSunVisible(bool);
	void	setMoonVisible(bool);
	void	setPlanetsVisible(bool);

	void	toggleAltAzmGridVisible();
	void	toggleRaDecGridVisible();
	void	toggleConstellationsVisible();
	void	toggleConstellationLabelsVisible();
	void	toggleTargetVisible();
	void	toggleTelescopeVisible();
	void	toggleTooltipVisible();
	void	toggleLabelsVisible();
	void	toggleEclipticVisible();
	void	toggleMilkywayVisible();
	void	toggleHorizonVisible();
	void	toggleSunVisible();
	void	toggleMoonVisible();
	void	togglePlanetsVisible();

	void	showContextMenu(const QPoint& point);

	void	rotationChanged(int angle);
	void	timeoffsetChanged(int tens);

	void	selectHorizon();

signals:
	void	pointSelected(astro::RaDec);
};

} // namespace snowgui 

#endif /* _SkyDisplayWidget_h */
