/*
 * StarChartWidget.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _StarChartWidget_h
#define _StarChartWidget_h

#include <QWidget>
#include <AstroCoordinates.h>
#include <AstroCatalog.h>

namespace snowgui {

/**
 * \brief A widget to display a chart of a window into the sky 
 */
class StarChartWidget : public QWidget {
	Q_OBJECT

	astro::catalog::Catalog::starsetptr	_stars;
	astro::Angle	_resolution;	// angle per pixel
	astro::RaDec	_direction;
	astro::ImageCoordinates	_converter;
	float	_limit_magnitude;
	bool	_negative;
	bool	_show_grid;

	QPointF	_center;
	QPointF	convert(const astro::RaDec& radec);
public:
	const astro::Angle&	resolution() const { return _resolution; }
	void	resolution(const astro::Angle& r) { _resolution = r; }

	void	limit_magnitude(float l) { _limit_magnitude = l; }
	float	limit_magnitude() const { return _limit_magnitude; }

	void	negative(bool n) { _negative = n; }
	bool	negative() const { return _negative; }

	void	show_grid(bool g) { _show_grid = g; }
	bool	show_grid() const { return _show_grid; }

	explicit StarChartWidget(QWidget *parent = NULL);
	virtual ~StarChartWidget();

signals:
	void	pointSelected(astro::RaDec);

private:
	void	draw();
	void	drawStar(QPainter& painter, const astro::catalog::Star& star);
	void	drawLine(QPainter& painter, const astro::RaDec& from,
			const astro::RaDec& to);
	void	drawGrid(QPainter& painter);

	void	mouseCommon(QMouseEvent *event);

protected:
	void	paintEvent(QPaintEvent *event);
	void	mousePressEvent(QMouseEvent *event);
	void	mouseMoveEvent(QMouseEvent *event);

public slots:
	void	directionChanged(astro::RaDec);
};

} // namespace snowgui

#endif /* _StarChartWidget_h */
