/*
 * Application.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "Application.h"
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <AstroFormat.h>

namespace snowgui {

bool	Application::notify(QObject *receiver, 	QEvent *event) {
	try {
		return QApplication::notify(receiver, event);
	} catch (const std::exception& x) {
		std::string	classname = astro::demangle(typeid(x).name());
		std::string	msg = astro::stringprintf("exception '%s' "
			"caught: %s", classname.c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	}
	return false;
}

} // namespace snowgui
