/*
 * Restart.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <Restart.h>
#include <unistd.h>
#include <AstroDebug.h>
#include <cstdlib>
#include <cerrno>
#include <cstring>

namespace snowstar {

bool	Restart::_shutdown_instead = false;

Restart::Restart(int argc, char *argv[]) {
	arguments = new char*[argc + 1];
	for (int i = 0; i < argc; i++) {
		arguments[i] = strdup(argv[i]);
	}
	arguments[argc] = NULL;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "prepared for restart (%d args)",
		argc);
}

void	Restart::shutdown_instead(bool s) {
	_shutdown_instead = s;
}

void	Restart::exec() {
	if (_shutdown_instead) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "shutdown requested instead");
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "restarting process");
	int	rc = execve(arguments[0], arguments, NULL);
	debug(LOG_ERR, DEBUG_LOG, 0, "restart failed: %s", strerror(errno));
}
	
} // namespace snowstar
