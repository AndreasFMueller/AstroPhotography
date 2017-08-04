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
 * \brief A Widget to display multiple data channels
 */	
class ChannelDisplayWidget : public QWidget {
	Q_OBJECT

	ChannelDataVector	_channels;
	std::vector<QColor>	_colors;

private:
	void	draw();
	double	channelMin(int channelid);
	double	channelMax(int channelid);
	double	allMin();
	double	allMax();

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
