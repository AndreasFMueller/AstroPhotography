//
// RotateDial.cpp
//
// (c) 2019 Prof Dr Andreas Müller, Hochschule Rapperswil
//
#include <RotateDial.h>
#include <QPainter>

namespace snowgui {

RotateDial::RotateDial(QWidget *parent) : QDial(parent) {
        setWrapping(true);
	setNotchesVisible(false);
	setMinimum(0);
	setMaximum(360);
	setSingleStep(1);
	setValue(180);
	resize(QSize(50, 50));
}

void	RotateDial::paintEvent(QPaintEvent *event) {
	QDial::paintEvent(event);
	// get a painter to paint on the buton
	QPainter	painter(this);
	int	deg = (value() + 180) % 360;
	char	buffer[10];
	snprintf(buffer, sizeof(buffer), "%d°", deg);
	QPen	pen(Qt::SolidLine);
	pen.setColor(Qt::black);
	painter.setPen(pen);
	painter.drawText(size().width() / 2 - 15,
		size().height() / 2 - 10, 30, 20,
		Qt::AlignCenter, QString(buffer));
}

} // namespace snowgui

