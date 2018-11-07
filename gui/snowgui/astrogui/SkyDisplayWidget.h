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
#include <QTimer>
#include <SkyDrawing.h>

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
class SkyDisplayWidget : public QWidget, public SkyDrawing {
	Q_OBJECT
	QTimer	_timer;
	bool	_show_tooltip;
public:
	bool	show_tooltip() const { return _show_tooltip; }
	void	show_tooltip(bool l) { _show_tooltip = l; }

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

	void	setAltAzmGridVisible(bool);
	void	setRaDecGridVisible(bool);
	void	setConstellationsVisible(bool);
	void	setTargetVisible(bool);
	void	setTelescopeVisible(bool);
	void	setTooltipVisible(bool);
	void	setLabelsVisible(bool);
	void	setEclipticVisible(bool);

	void	toggleAltAzmGridVisible();
	void	toggleRaDecGridVisible();
	void	toggleConstellationsVisible();
	void	toggleTargetVisible();
	void	toggleTelescopeVisible();
	void	toggleTooltipVisible();
	void	toggleLabelsVisible();
	void	toggleEclipticVisible();

	void	showContextMenu(const QPoint& point);

signals:
	void	pointSelected(astro::RaDec);
};

} // namespace snowgui 

#endif /* _SkyDisplayWidget_h */
