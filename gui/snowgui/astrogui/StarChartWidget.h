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
#include <StarChartLegend.h>
#include <ImagerRectangle.h>

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
	void	deepskyReady(astro::catalog::DeepSkyObjectSetPtr);
};

/**
 * \brief A widget to display a chart of a window into the sky 
 *
 * XXX desirable features: allow for the display of a rectangle of the
 * XXX size of the CCD chip. The difficulty is how how to select the
 * XXX from the three sizes of guider, finder and imager
 */
class StarChartWidget : public QWidget {
	Q_OBJECT

	astro::catalog::Catalog::starsetptr	_stars;
	astro::catalog::Catalog::starsetptr	_sky;
	astro::catalog::DeepSkyObjectSetPtr	_deepsky;
	astro::catalog::OutlineCatalogPtr	_outlines;
	astro::Angle	_resolution;	// angle per pixel
	astro::Angle	_imager_resolution;
	astro::Angle	_finder_resolution;
	astro::Angle	_guider_resolution;
	snowgui::ImagerRectangle	_imager_rectangle;
	snowgui::ImagerRectangle	_finder_rectangle;
	snowgui::ImagerRectangle	_guider_rectangle;
	astro::RaDec	_direction;
	astro::RaDec	_chartcenter;
	astro::device::Mount::state_type	_state;
	astro::ImageCoordinates	_converter;
	float	_limit_magnitude;
	bool	_negative;
	bool	_show_stars;
	bool	_show_grid;
	bool	_show_crosshairs;
	bool	_show_directions;
	bool	_show_deepsky;
	bool	_show_tooltips;
	bool	_show_cataloglabels;
	bool	_flip;
	bool	_show_imager_rectangle;
	bool	_show_finder_rectangle;
	bool	_show_guider_rectangle;

	bool	_retrieval_necessary;
	StarChartRetriever	*_retriever;
	BusyWidget	*_busywidget;

	StarChartLegend	*_legend;

	QPointF	_center;
	QPointF	convert(const astro::RaDec& radec);
public:
	const astro::Angle&	resolution() const { return _resolution; }
	void	resolution(const astro::Angle& r) { _resolution = r; }

	void	limit_magnitude(float l) { _limit_magnitude = l; }
	float	limit_magnitude() const { return _limit_magnitude; }

	void	negative(bool n) { _negative = n; }
	bool	negative() const { return _negative; }

	void	show_stars(bool s) { _show_stars = s; }
	bool	show_stars() const { return _show_stars; }

	void	show_grid(bool g) { _show_grid = g; }
	bool	show_grid() const { return _show_grid; }

	void	show_crosshairs(bool c) { _show_crosshairs = c; }
	bool	show_crosshairs() const { return _show_crosshairs; }

	void	show_directions(bool d) { _show_directions = d; }
	bool	show_directions() const { return _show_directions; }

	void	show_deepsky(bool d) { _show_deepsky = d; }
	bool	show_deepsky() const { return _show_deepsky; }

	void	show_tooltips(bool d) { _show_tooltips = d; }
	bool	show_tooltips() const { return _show_tooltips; }

	void	show_cataloglabels(bool d) { _show_cataloglabels = d; }
	bool	show_cataloglabels() const { return _show_cataloglabels; }

	void	flip(bool f) { _flip = f; }
	bool	flip() const { return _flip; }

	void	show_imager_rectangle(bool r) { _show_imager_rectangle = r; }
	bool	show_imager_rectangle() const { return _show_imager_rectangle; }

	void	show_finder_rectangle(bool r) { _show_finder_rectangle = r; }
	bool	show_finder_rectangle() const { return _show_finder_rectangle; }

	void	show_guider_rectangle(bool r) { _show_guider_rectangle = r; }
	bool	show_guider_rectangle() const { return _show_guider_rectangle; }

	explicit StarChartWidget(QWidget *parent = NULL);
	virtual ~StarChartWidget();

signals:
	void	pointSelected(astro::RaDec);

private:
	void	draw();
	void	drawStar(QPainter& painter, const astro::catalog::Star& star);
	void	drawStars(QPainter& painter);
	void	drawDeepSkyObject(QPainter& painter,
			const astro::catalog::DeepSkyObject& deepskyobject);
	void	drawLine(QPainter& painter, const astro::RaDec& from,
			const astro::RaDec& to);
	void	drawGrid(QPainter& painter);
	void	drawCrosshairs(QPainter& painter);
	void	drawDirections(QPainter& painter);
	QPointF	rectanglePoint(const astro::RaDec&);
	void	drawRectangle(QPainter& painter,
			const ImagerRectangle& rectangle,
			const astro::Angle& resolution);

	void	mouseCommon(QMouseEvent *event);

	bool	_mouse_pressed;

private slots:
	void	startRetrieval();

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
	void	useDeepSky(astro::catalog::DeepSkyObjectSetPtr);
	void	workerFinished();
	void	guiderResolution(astro::Angle);
	void	finderResolution(astro::Angle);
	void	imagerResolution(astro::Angle);
	void	resolutionChanged(astro::Angle);
	void	guiderRectangle(snowgui::ImagerRectangle);
	void	finderRectangle(snowgui::ImagerRectangle);
	void	imagerRectangle(snowgui::ImagerRectangle);

	void	setStarsVisible(bool);
	void	setGridVisible(bool);
	void	setCrosshairsVisible(bool);
	void	setDirectionsVisible(bool);
	void	setDeepskyVisible(bool);
	void	setCataloglabelsVisible(bool);
	void	setTooltipsVisible(bool);
	void	setNegative(bool);
	void	setFlip(bool);
	void	setImagerRectangleVisible(bool);
	void	setFinderRectangleVisible(bool);
	void	setGuiderRectangleVisible(bool);
	
	void	toggleStarsVisible();
	void	toggleGridVisible();
	void	toggleCrosshairsVisible();
	void	toggleDirectionsVisible();
	void	toggleDeepskyVisible();
	void	toggleCataloglabelsVisible();
	void	toggleTooltipsVisible();
	void	toggleNegative();
	void	toggleFlip();
	void	toggleImagerRectangleVisible();
	void	toggleFinderRectangleVisible();
	void	toggleGuiderRectangleVisible();

	void	useFinderResolution();
	void	useGuiderResolution();
	void	useImagerResolution();
	void	useStandardResolution();

	void	showLegend();
	void	removeLegend();
	
	void    showContextMenu(const QPoint& point);
};

} // namespace snowgui

#endif /* _StarChartWidget_h */
