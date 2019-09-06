/*
 * StarChartLegend.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <StarChartLegend.h>
#include <QPainter>
#include <AstroDebug.h>

namespace snowgui {

const static int	h = 10;
const static int	r = 5;
const static int	b = 3;

StarChartLegend::StarChartLegend(QWidget *parent) : QWidget(parent) {
	resize(260, 2 * (b + 6 * h));
	setFixedSize(size());
	setWindowTitle(QString("Deep sky object color key"));
}

StarChartLegend::~StarChartLegend() {
}

void	StarChartLegend::closeEvent(QCloseEvent * /* event */) {
	deleteLater();
}

void	StarChartLegend::draw(QPainter& painter, int& y, QColor color,
		QString label) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draing '%s'", label.toLatin1().data());
	painter.setRenderHint(QPainter::Antialiasing);

        QPen	pen(Qt::SolidLine);
        pen.setWidth(1);
	pen.setColor(color);
	painter.setPen(pen);
	painter.drawText(2 * h + b, y - h, 240, 2 * h,
		Qt::AlignLeft | Qt::AlignVCenter, label);

	QPainterPath	path;
	path.addEllipse(h - r + b, y - r, 2 * r, 2 * r);
	painter.fillPath(path, color);

	y += 2 * h;
}

void	StarChartLegend::paintEvent(QPaintEvent * /* event */) {
	QPainter	painter(this);

	painter.fillRect(0, 0, size().width(), size().height(), Qt::black);

	int	y = h + b;

	draw(painter, y, Qt::red,     QString("Galaxy"));
	draw(painter, y, Qt::green,   QString("Bright nebula, Cluster with nebulosity"));
	draw(painter, y, Qt::magenta, QString("Planetary nebula"));
	draw(painter, y, Qt::yellow,  QString("Globular cluster"));
	draw(painter, y, Qt::cyan,    QString("Open cluster"));
	draw(painter, y, Qt::gray,    QString("undefined"));
}

} // namespace snowgui
