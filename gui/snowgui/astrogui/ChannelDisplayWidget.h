/*
 * ChannelDisplayWidget.h -- widget that can display a history of values
 *                           from multiple channels
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ChannelDisplayWidget_h
#define _ChannelDisplayWidget_h

#include <QWidget>
#include <vector>
#include <cmath>
#include <AstroUtils.h>
#include "ChannelData.h"

namespace snowgui {

/**
 * \brief Class to draw a single channel
 */
class ChannelPainter {
	double		_notbefore;
	double		_notafter;
	double		_yscale;
	double		_tscale;
	double		_width;
	double		_height;
public:
	double	notbefore() const { return _notbefore; }
	double	notafter() const { return _notafter; }
	double	yscale() const { return _yscale; }
	double	width() const { return _width; }
	double	height() const { return _height; }
	void	notbefore(double x) { _notbefore = x; }
	void	notafter(double x) { _notafter = x; }
	void	yscale(double x) { _yscale = x; }
	void	width(double x) { _width = x; }
	void	height(double x) { _height = x; }

	double	X(double t) const;
	double	Y(double y) const;
	QPoint	P(double t, double y) const;
private:
	QPainter&	_painter;
public:
	ChannelPainter(QPainter& painter) : _painter(painter) { }
	void	operator()(const ChannelData& channel, const QColor& color);
	void	operator()(const ChannelDataVector& channels,
			const std::vector<QColor>& colors);
};

/**
 * \brief A Widget to display multiple data channels
 */	
class ChannelDisplayWidget : public QWidget {
	Q_OBJECT

	ChannelDataVector	_channels;
	std::vector<QColor>	_colors;

	bool	_autorange;
public:
	bool	autorange() const { return _autorange; }
	void	setAutorange(bool a) { _autorange = a; }

private:
	double	_timescale;
public:
	double	timescale() const { return _timescale; }
	void	setTimescale(double t) { _timescale = t; }

private:
	void	draw(double notbefore, double notafter);

public: 
	explicit ChannelDisplayWidget(QWidget *parent = NULL);
	virtual ~ChannelDisplayWidget();

	int	channels() const;
	void	addChannel(QColor color);
	void	update(int channel, ChannelDataPoint p);

	void	paintEvent(QPaintEvent *event);

	void	clearData();

public slots:
	void	add(double time, std::vector<double> values);

private slots:

};

} // namespace snowgui

#endif /* _ChannelDisplayWidget_h */
