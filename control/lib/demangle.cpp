/*
 * demangle.cpp -- demangle symbol names
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cxxabi.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <cstdlib>

namespace astro {

/**
 * \brief Demangle a C++ name to human readable form
 *
 * This function uses the __cxa_demangle function from namespace abi
 * to convert a mangled name into something human readable.
 */
std::string	demangle(const std::string& mangled_name) {
	int	status = 0;
	char	*demangled = abi::__cxa_demangle(mangled_name.c_str(), NULL, 0,
			&status);
	if (NULL == demangled) {
		std::string	msg = stringprintf("demangling failed: %d",
			status);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		return mangled_name;
	}
	std::string	result(demangled);
	free(demangled);
	return result;
}

} // namespace astro
