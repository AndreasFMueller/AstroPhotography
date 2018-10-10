/*
 * LocalSiderealTime.cpp -- Widget to continuously display LMST
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "LocalSiderealTime.h"

namespace snowgui {

/**
 * \brief Construct a local sidereal time clock
 */
LocalSiderealTime::LocalSiderealTime(QWidget *parent) : QLabel(parent) {
	_offset = 0;
	_timer.setInterval(1000);
	connect(&_timer, SIGNAL(timeout()), this, SLOT(update()));
	_timer.start();
}

/**
 * \brief Destroy the local sidereal time clock
 */
LocalSiderealTime::~LocalSiderealTime() {
	_timer.stop();
}

/**
 * \brief Common update
 *
 * This method does not take the offset into account
 *
 * \param now	this is the absolute point in time for which LMST is desired
 */
void	LocalSiderealTime::updateCommon(time_t now) {
	// compute the LMST and display
	astro::AzmAltConverter	c(now, _position);
	std::string	t = c.LMST().hms().substr(1);
	size_t	p = t.find('.');
	if (p != std::string::npos) {
		t = t.substr(0, p);
	}
	setText(QString(t.c_str()));
}

/**
 * \brief Update the local sidereal time clock
 */
void	LocalSiderealTime::update() {
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
void	LocalSiderealTime::update(time_t now) {
	time_t	localnow;
	time(&localnow);
	_offset = now - localnow;
	updateCommon(now);
}

} // namespace snowgui
