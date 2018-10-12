/*
 * StarChartWidget.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _StarChartWidget_h
#define _StarChartWidget_h

#include <QWidget>
#include <QThread>
#include <AstroCoordinates.h>
#include <AstroCatalog.h>
#include <AstroDevice.h>
#include <BusyWidget.h>

namespace snowgui {

/**
 * \brief Worker class to retrieve sets of stars
 */
class StarChartRetriever : public QThread {
	Q_OBJECT
	void	run() override;
	float	_limit_magnitude;
	astro::catalog::SkyWindow	_window;
public:
	void	limit_magnitude(float l) { _limit_magnitude = l; }
	float	limit_magnitude() const { return _limit_magnitude; }

	const astro::catalog::SkyWindow& window() const { return _window; }
	void	window(const astro::catalog::SkyWindow& w) { _window = w; }

	StarChartRetriever(QObject *parent = NULL);
signals:
	void	starsReady(astro::catalog::Catalog::starsetptr);
};

/**
 * \brief Worker class to retrieve the deep sky objects in a window
 */
class DeepSkyRetriever : public QThread {
	Q_OBJECT
	void	run() override;
	astro::catalog::SkyWindow	_window;

public:
	const astro::catalog::SkyWindow& window() const { return _window; }
	void	window(const astro::catalog::SkyWindow& w) { _window = w; }

	DeepSkyRetriever(QObject *parent = NULL);
signals:
	void	deepskyReady(astro::catalog::DeepSkyCatalog::deepskyobjectsetptr);
};

/**
 * \brief A widget to display a chart of a window into the sky 
 */
class StarChartWidget : public QWidget {
	Q_OBJECT

	astro::catalog::Catalog::starsetptr	_stars;
	astro::catalog::Catalog::starsetptr	_sky;
	astro::catalog::DeepSkyCatalog::deepskyobjectsetptr	_deepsky;
	astro::Angle	_resolution;	// angle per pixel
	astro::RaDec	_direction;
	astro::device::Mount::state_type	_state;
	astro::ImageCoordinates	_converter;
	float	_limit_magnitude;
	bool	_negative;
	bool	_show_grid;
	bool	_show_crosshairs;
	bool	_show_directions;
	bool	_show_deepsky;
	bool	_flip;

	bool	_retrieval_necessary;
	StarChartRetriever	*_retriever;
	BusyWidget	*_busywidget;

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

	void	show_crosshairs(bool c) { _show_crosshairs = c; }
	bool	show_crosshairs() const { return _show_crosshairs; }

	void	show_directions(bool d) { _show_directions = d; }
	bool	show_directions() const { return _show_directions; }

	void	show_deepsky(bool d) { _show_deepsky = d; }
	bool	show_deepsky() const { return _show_deepsky; }

	void	flip(bool f) { _flip = f; }
	bool	flip() const { return _flip; }

	explicit StarChartWidget(QWidget *parent = NULL);
	virtual ~StarChartWidget();

signals:
	void	pointSelected(astro::RaDec);

private:
	void	draw();
	void	drawStar(QPainter& painter, const astro::catalog::Star& star);
	void	drawDeepSkyObject(QPainter& painter,
			const astro::catalog::DeepSkyObject& deepskyobject);
	void	drawLine(QPainter& painter, const astro::RaDec& from,
			const astro::RaDec& to);
	void	drawGrid(QPainter& painter);
	void	drawCrosshairs(QPainter& painter);
	void	drawDirections(QPainter& painter);

	void	mouseCommon(QMouseEvent *event);

	void	startRetrieval();

	bool	_mouse_pressed;
protected:
	void	paintEvent(QPaintEvent *event);
	void	mousePressEvent(QMouseEvent *event);
	void	mouseReleaseEvent(QMouseEvent *event);
	void	mouseMoveEvent(QMouseEvent *event);

public slots:
	void	directionChanged(astro::RaDec);
	void	orientationChanged(bool west);
	void	stateChanged(astro::device::Mount::state_type);
	void	useStars(astro::catalog::Catalog::starsetptr);
	void	useSky(astro::catalog::Catalog::starsetptr);
	void	useDeepSky(astro::catalog::DeepSkyCatalog::deepskyobjectsetptr);
	void	workerFinished();
};

} // namespace snowgui

#endif /* _StarChartWidget_h */
