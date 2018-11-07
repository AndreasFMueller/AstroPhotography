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
#include <AstroCatalog.h>
#include <AstroCoordinates.h>
#include <set>
#include <cmath>

namespace snowgui {

class SkyDrawing {
	Q_GADGET
	astro::catalog::Catalog::starsetptr	_stars;
protected:
	astro::AzmAltConverter	*_converter;
private:
	bool	_show_altaz;
	bool	_show_radec;
	bool	_show_ecliptic;
	bool	_show_constellations;
	bool	_show_telescope;
	bool	_show_target;
	bool	_show_labels;
	astro::RaDec	_telescope;
	astro::RaDec	_target;
	astro::LongLat	_position;
public:
	explicit	SkyDrawing();
	virtual ~SkyDrawing();
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
	void	telescope(const astro::RaDec& t) { _telescope = t; }
	const astro::RaDec&	telescope() const { return _telescope; }
	void	target(const astro::RaDec& t) { _target = t; }
	const astro::RaDec&	target() const { return _target; }
	void	position(const astro::LongLat& p) { _position = p; }
        const astro::LongLat&	position() const { return _position; }

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
	void	drawAltaz(QPainter& painter);
	void	drawRadec(QPainter& painter);
	void	drawEcliptic(QPainter& painter);
	void	drawTarget(QPainter& painter);
	void	drawConstellations(QPainter& painter);
	void	drawLabels(QPainter& painter);
protected:
	virtual void	redraw();
	astro::AzmAlt	convert(const astro::RaDec& radec);
	QPointF	convert(const astro::AzmAlt& azmalt);
public:
	void	draw(QPainter& painter, QSize& size);
};

} // namespace snowgui

#endif /* _SkyDrawing_h */
