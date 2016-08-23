/*
 * ChannelDisplayWidget.h -- widget that can display a history of values
 *                           from multiple channels
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ChannelDisplayWidget_h
#define _ChannelDisplayWidget_h

#include <QWidget>
#include <deque>
#include <vector>
#include <cmath>

namespace snowgui {

class ChannelDisplayWidget : public QWidget {
	Q_OBJECT

	std::vector<std::deque<double> >	_channels;
	std::vector<QColor>			_colors;

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
	void	update(int channel, double value);

	void	paintEvent(QPaintEvent *event);

public slots:
	void	add(std::vector<double> values);

private slots:

};

} // namespace snowgui

#endif /* _ChannelDisplayWidget_h */
