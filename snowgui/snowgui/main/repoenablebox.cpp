/*
 * repoenablebox.cpp
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "repoenablebox.h"
#include <AstroDebug.h>

namespace snowgui {

/**
 * \brief Construct a new repositoryconfiguration widget
 */
repoenablebox::repoenablebox(QWidget *parent) : QCheckBox(parent) {
}

/**
 * \brief Destroy the repositoryconfiguration widget
 */
repoenablebox::~repoenablebox() {
}

/**
 * \brief Set the repositories proxy
 */
void	repoenablebox::setRepositories(snowstar::RepositoriesPrx repositories) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got repository proxy");
	_repositories = repositories;
}

/**
 * \brief Slot to handle state change
 */
void	repoenablebox::enableToggled(bool enabled) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "toggle repository '%s' to %s",
		_reponame.c_str(), (enabled) ? "enabled" : "disabled");
	try {
		_repositories->setHidden(_reponame, (enabled) ? false : true);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"setHidden(%s, %s) caused exception: %s",
			_reponame.c_str(), (enabled) ? "false" : "true",
			x.what());
	}
}

}
