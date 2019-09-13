//
// OffsetDial.cpp
//
// (c) 2019 Prof Dr Andreas Müller, Hochschule Rapperswil
//
#include <OffsetDial.h>
#include <QPainter>

namespace snowgui {

OffsetDial::OffsetDial(QWidget *parent) : QDial(parent) {
	setNotchesVisible(false);
	setMinimum(-720);
	setMaximum(720);
	setSingleStep(15);
	setValue(0);
	resize(QSize(50, 50));
}

void	OffsetDial::paintEvent(QPaintEvent * event) {
	QDial::paintEvent(event);
	// get a painter to paint on the buton
	QPainter	painter(this);
	int	tens = value();
	int	hours = tens / 60;
	int	mins = (tens - hours * 60);
	if (mins < 0) { mins = -mins; }
	char	buffer[10];
	snprintf(buffer, sizeof(buffer), "%c%d:%02d", (tens < 0) ? '-' : '+',
		(hours < 0) ? -hours : hours, mins);
	QPen	pen(Qt::SolidLine);
	pen.setColor(Qt::black);
	painter.setPen(pen);
	painter.drawText(size().width() / 2 - 25,
		size().height() / 2 - 10, 50, 20,
		Qt::AlignCenter, QString(buffer));
}

} // namespace snowgui

