/*
 * ChannelPainter.cpp -- implementation of the channel painter class
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <ChannelDisplayWidget.h>
#include <QPainter>


namespace snowgui {

void	ChannelPainter::notbefore(double x) {
	_notbefore = x;
	_tscale = (notafter() - notbefore()) / width();
}

void	ChannelPainter::notafter(double x) {
	_notafter = x;
	_tscale = (notafter() - notbefore()) / width();
}

void	ChannelPainter::width(double x) {
	_width = x;
	_tscale = (notafter() - notbefore()) / width();
}


double	ChannelPainter::X(double t) const {
	return width() - (notafter() - t) * _tscale;
}

double	ChannelPainter::Y(double y) const {
	return height() / 2 - 1 - yscale() * y;
}

QPoint	ChannelPainter::P(double t, double y) const {
	return QPoint(X(t), Y(y));
}

void	ChannelPainter::operator()(const ChannelData& channel,
	const QColor& color) {
	QPen	pen(Qt::SolidLine);
        pen.setWidth(1);
        pen.setColor(color);
	_painter.setPen(pen);

	ChannelData::const_reverse_iterator	r;
	r = channel.crbegin();
	int	w = 1;
	QPoint	p = P(r->time, r->value);
	r++; w++;
	do {
		QPoint	q = P(r->time, r->value);
		_painter.drawLine(p, q);
		p = q;
		r++; w++;
	} while ((w <= width()) && (r != channel.crend()));
}

void    ChannelPainter::operator()(const ChannelDataVector& channels, 
                        const std::vector<QColor>& colors) {
	for (unsigned long i = 0; i < channels.size(); i++) {
		(*this)(channels[i], colors[i]);
	}
}

} // namespace snowgui
