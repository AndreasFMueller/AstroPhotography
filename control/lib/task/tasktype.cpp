/*
 * tasktype.cpp
 *
 * (c) 2019 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTask.h>
#include <AstroFormat.h>

namespace astro {
namespace task {

tasktype::tasktype(int t) {
	if ((t < 0) || (t > 3)) {
		std::string	msg = stringprintf("invalid tasktype %d", t);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	_t = (type)t;
}

std::string	tasktype::toString() const {
	switch (_t) {
	case EXPOSURE:
		return std::string("exposure");
	case DITHER:
		return std::string("dither");
	case FOCUS:
		return std::string("focus");
	case SLEEP:
		return std::string("sleep");
	}
	throw std::runtime_error("unknown type");
}

} // namespace task
} // namespace astro
