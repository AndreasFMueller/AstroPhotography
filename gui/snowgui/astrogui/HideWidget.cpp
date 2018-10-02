/*
 * HideWidget.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <HideWidget.h>
#include <QPainter>
#include <AstroDebug.h>
#include <QPalette>

namespace snowgui {

/**
 * \brief Construct a HideWidget
 */
HideWidget::HideWidget(QString text, QWidget *parent)
		: QWidget(parent), _text(text) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create the HideWidget");
	// the widget starts out completely transparent without the text
	setAutoFillBackground(true);
	QPalette pal;
	QColor	clear(0, 0, 0, 0);
	pal.setColor(QPalette::Background, clear);
	setPalette(pal);
	_hide = false;

	// start the timer that will hide the base widget as soon as
	// it expires after 1 second
	_timer = new QTimer();
	_timer->setInterval(500);
	_timer->setSingleShot(true);
	connect(_timer, SIGNAL(timeout()), this, SLOT(timeout()));
	_timer->start();
}

/**
 * \brief Destroy the widget
 */
HideWidget::~HideWidget() {
	_timer->stop();
	delete _timer;
}

/**
 * \brief Set the text displayed in the middle of the widget
 */
void	HideWidget::setText(QString t) {
	_text = t;
	repaint();
}

/**
 * \brief Draw the contents
 */
void	HideWidget::draw() {
	if (!_hide) {
		return;
	}
	QPainter	painter(this);
	QPen	pen;
        QColor  white(255,255,255);
        pen.setColor(white);
        painter.setPen(pen);
        painter.drawText(0, 0, width(), height(), Qt::AlignCenter, _text);
}

/**
 * \brief Handle the paint event
 */
void	HideWidget::paintEvent(QPaintEvent * /* event */) {
	draw();
}

/**
 * \brief Handle the timeout
 */
void	HideWidget::timeout() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "hiding");
	QPalette pal;
	QColor	mask(0, 0, 0, 100);
	pal.setColor(QPalette::Background, mask);
	setPalette(pal);
	_hide = true;
	repaint();
}

} // namespace snowgui
