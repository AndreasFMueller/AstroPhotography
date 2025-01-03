/*
 * HideProgress.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "HideProgress.h"
#include <QProgressBar>
#include <QResizeEvent>
#include <cmath>

namespace snowgui {

/**
 * \brief Construct a progressbar
 *
 * \param duration	duration of the exposure
 * \param parent	parent widget
 */
HideProgress::HideProgress(float duration, QWidget *parent)
	: QWidget(parent), _duration(duration) {
	// make the widget transparent
	setAutoFillBackground(true);
	QPalette	pal;
	QColor	mask(0, 0, 0, 50);
	pal.setColor(QPalette::Window, mask);
	setPalette(pal);

	// create the progress bar
	_progressbar = new QProgressBar(this);
	_progressbar->setMinimum(0);
	_progressbar->setMaximum(100);
	_progressbar->setGeometry(0, 0, width(), 10);
	_progressbar->setVisible(true);

	// start the timer
	connect(&_timer, SIGNAL(timeout()), this, SLOT(update()));
	_timer.setInterval(100);
	_timer.start();
	_start.start();
}

/**
 * \brief Destroy the progress bar
 */
HideProgress::~HideProgress() {
	_timer.stop();
	disconnect(&_timer, SIGNAL(timeout()), 0, 0);
	delete _progressbar;
}

/**
 * \brief Handle a resize event, the new size must be propagated
 *
 * \param event		resize event
 */
void	HideProgress::resizeEvent(QResizeEvent *event) {
	_progressbar->setGeometry(0, 0, event->size().width(), 10);
}

/**
 * \brief Timer Update 
 */
void	HideProgress::update() {
	double	timepast = _start.gettime() - _start.startTime();
	int	p = trunc(100 * timepast / _duration);
	if (p > 100) {
		p = 100;
	}
	if (p < 0) {
		p = 0;
	}
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure value: %d", p);
	_progressbar->setValue(p);
}


} // namespace snowgui
