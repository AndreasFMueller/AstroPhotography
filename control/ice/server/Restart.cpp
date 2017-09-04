/*
 * Restart.cpp -- implementation of the restart object
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <Restart.h>
#include <unistd.h>
#include <AstroDebug.h>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <AstroEvent.h>
#include <sstream>

namespace snowstar {

bool	Restart::_shutdown_instead = false;

/**
 * \brief Construct the Restart object
 *
 * The constructor remembers the arguments so that the process can be
 * restarted.
 */
Restart::Restart(int argc, char *argv[]) {
	arguments = new char*[argc + 1];
	for (int i = 0; i < argc; i++) {
		arguments[i] = strdup(argv[i]);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "arguments[%d] = '%s'", i,
			arguments[i]);
	}
	arguments[argc] = NULL;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "prepared for restart (%d args)",
		argc);
}

/**
 * \brief Set the shutdown instead flag
 */
void	Restart::shutdown_instead(bool s) {
	_shutdown_instead = s;
}

/**
 * \brief Perform a restart
 */
void	Restart::exec() {
	char	path[PATH_MAX];
	getcwd(path, sizeof(path));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Restart::exec(), path: %s", path);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", toString().c_str());

	// if shutdown is requested, do that
	if (_shutdown_instead) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "shutdown requested instead");
		astro::event(__FILE__, __LINE__, "snowstar::Restart",
			astro::events::WARNING,
			astro::events::Event::SERVER,
			"server is shutting down");
		return;
	}

	// record the event that we restart the server
	astro::event(__FILE__, __LINE__, "snowstar::Restart",
		astro::events::WARNING,
		astro::events::Event::SERVER,
		"server is restarting now");

	// naw actually restart the process
	debug(LOG_DEBUG, DEBUG_LOG, 0, "restarting process");
	int	rc = execve(arguments[0], arguments, NULL);
	debug(LOG_ERR, DEBUG_LOG, 0, "cannot restart: %d, %s", rc,
		strerror(errno));
}

/**
 * \brief convert the arguments 
 */
std::string	Restart::toString() const {
	std::ostringstream	out;
	int	i = 0;
	for (char **p = arguments; *p; p++, i++) {
		out << "arguments[" << i << "] = '" << *p << "'" << std::endl;
	}
	return out.str();
}

/**
 * \brief Output the contents of the arguments to a stream
 */
std::ostream&	operator<<(std::ostream& out, const Restart& restart) {
	out << restart.toString();
	return out;
}
	
} // namespace snowstar
