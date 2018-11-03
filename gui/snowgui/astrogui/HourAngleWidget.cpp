/*
 * HourAngleWidget.cpp -- Widget to continuously display LMST
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "HourAngleWidget.h"

namespace snowgui {

/**
 * \brief Construct a local sidereal time clock
 */
HourAngleWidget::HourAngleWidget(QWidget *parent) : QLabel(parent) {
	_offset = 0;
	_timer.setInterval(1000);
	connect(&_timer, SIGNAL(timeout()), this, SLOT(update()));
	_timer.start();
}

/**
 * \brief Destroy the local sidereal time clock
 */
HourAngleWidget::~HourAngleWidget() {
	_timer.stop();
}

/**
 * \brief Common update
 *
 * This method does not take the offset into account
 *
 * \param now	this is the absolute point in time for which LMST is desired
 */
void	HourAngleWidget::updateCommon(time_t now) {
	// compute the LMST and display
	astro::AzmAltConverter	c(now, _position);
	astro::Angle	hourangle = c.LMST() - _ra;
	if (hourangle < astro::Angle(-M_PI)) {
		hourangle = hourangle = astro::Angle(2 * M_PI);
	}
	std::string	t = hourangle.hms(':', 0).substr(1);
	setText(QString(t.c_str()));
}

/**
 * \brief Update the local sidereal time clock
 */
void	HourAngleWidget::update() {
	// compute the time
	time_t	now;
	time(&now);
	now += _offset;

	// common update call
	updateCommon(now);
}

/**
 * \brief Slot for updates with time
 *
 * This slot implicitly updates the time offset so that the widget keeps
 * displaying the time with the same time offset
 */
void	HourAngleWidget::update(time_t now) {
	time_t	localnow;
	time(&localnow);
	_offset = now - localnow;
	updateCommon(now);
}

/**
 * \brief Set the position
 */
void    HourAngleWidget::position(const astro::LongLat& p) {
	_position = p;
	update();
}

void	HourAngleWidget::ra(const astro::Angle& r) {
	_ra = r;
	update();
}

} // namespace snowgui
