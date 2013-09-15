/*
 * histogramwidget.h -- Widget to display a histogram
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef HISTOGRAMWIDGET_H
#define HISTOGRAMWIDGET_H

#include <QWidget>
#include <AstroHistogram.h>

using namespace astro::image;

class HistogramWidget : public QWidget
{
    Q_OBJECT

	// the histogram we have to display
	HistogramSet	histogramset;
	unsigned int	maxcount;

	// whether or not to use a logarithmic display format
	bool	logarithmic;
	bool	color;
	void	update();
	double	_min;
	double	_max;
	double	_minmark;
	double	_maxmark;
public:
    explicit HistogramWidget(QWidget *parent = 0);
    virtual ~HistogramWidget();

    void	setHistograms(HistogramSet _histogramset);
    void	paintEvent(QPaintEvent *event);

	void	drawLuminance();
	void	drawColor();

	double	min() const { return _min; }
	double	max() const { return _max; }
	double	minmark() const { return _minmark; }
	double	maxmark() const { return _maxmark; }
	void	setMinmark(double minmark) { _minmark = minmark; }
	void	setMaxmark(double maxmark) { _maxmark = maxmark; }

private slots:
};

#endif // HISTOGRAMWIDGET_H
