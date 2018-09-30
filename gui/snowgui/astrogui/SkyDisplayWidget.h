/*
 * SkyDisplayWidget.h
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

namespace snowgui {

/**
 * \brief Widget to display the sky with stars and telescope marker
 */
class SkyDisplayWidget : public QWidget {
	Q_OBJECT
	astro::catalog::Catalog::starsetptr	_stars;

	astro::AzmAltConverter	*_converter;
	astro::AzmAlt	convert(const astro::RaDec& radec);
	QPointF	_center;
	float	_radius;
	// whether or not to show the altaz grid lines
	bool	_show_altaz;
public:
	bool	show_altaz() const { return _show_altaz; }
	void	show_altaz(bool a) { _show_altaz = a; }

	// whether or not to show the right ascension and declination grid
private:
	bool	_show_radec;
public:
	bool	show_radec() const { return _show_radec; }
	void	show_radec(bool r) { _show_radec = r; }

	// telescope direction in right ascension and declination
private:
	astro::RaDec	_telescope;
public:
	void	telescope(const astro::RaDec& t) { _telescope = t; }
	const astro::RaDec&	telescope() const { return _telescope; }

	// position on earth of the observatory
private:
	astro::LongLat	_position;
public:
	void	position(const astro::LongLat& p) { _position = p; }
	const astro::LongLat&	position() const { return _position; }

	// some private drawing functions
private:
	void	drawStar(QPainter& painter, const astro::catalog::Star& star);
	void	drawTelescope(QPainter& painter);
	void	drawAltaz(QPainter& painter);
	void	drawRadec(QPainter& painter);
	void	draw();
	QPointF	convert(const astro::AzmAlt& azmalt);

	// constructors
public:
	explicit SkyDisplayWidget(QWidget *parent = NULL);
	virtual ~SkyDisplayWidget();

	void	paintEvent(QPaintEvent *event);

public slots:
	void	telescopeChanged(astro::RaDec);
	void	positionChanged(astro::LongLat);
};

} // namespace snowgui 

#endif /* _SkyDisplayWidget_h */
