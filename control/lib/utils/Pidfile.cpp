/*
 * Pidfile.cpp -- implementation of the PidFile class
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <includes.h>

namespace astro {

/**
 * \brief Create pid file
 */
PidFile::PidFile(const std::string& filename)
	: _filename(filename) {
	int	fd = open(filename.c_str(), O_RDWR | O_TRUNC | O_CREAT, 0644);
	if (fd < 0) {
		std::string	cause = stringprintf("cannot create pid file: "
			"%s: %s", filename.c_str(), strerror(errno));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw std::runtime_error(cause);
	}
	char	buffer[32];
	snprintf(buffer, sizeof(buffer), "%d\n", getpid());
	if (write(fd, buffer, strlen(buffer)) < 0) {
		std::string	cause = stringprintf("cannot create pid file: "
			"%s", strerror(errno));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		close(fd);
		throw std::runtime_error(cause);
	}
	close(fd);
}

/**
 * \brief Remove the pid file
 */
PidFile::~PidFile() {
	unlink(_filename.c_str());
}

} // namespace astro
