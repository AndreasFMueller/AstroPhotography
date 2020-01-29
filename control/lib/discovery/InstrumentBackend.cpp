/*
 * InstrumentBackend.cpp -- instrument implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */

#include "InstrumentBackendImpl.h"

namespace astro {
namespace discover {

InstrumentBackend::InstrumentBackend() {
}

InstrumentList	InstrumentBackend::names() {
	InstrumentBackendImpl	backend;
	return backend.names();
}

bool	InstrumentBackend::has(const std::string& name) {
	InstrumentBackendImpl	backend;
	return backend.has(name);
}

InstrumentPtr	InstrumentBackend::get(const std::string& name) {
	InstrumentBackendImpl	backend;
	return backend.get(name);
}

void	InstrumentBackend::remove(const std::string& name) {
	InstrumentBackendImpl	backend;
	backend.remove(name);
}

} // namespace discover
} // namespace astro
