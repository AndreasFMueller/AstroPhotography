/*
 * Camera_impl.cpp --
 *
 * (c) 2013 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <Camera_impl.h>

namespace Astro {

CORBA::Long	Camera_impl::nCcds() {
	return _camera->nCcds();
}

char	*Camera_impl::getName() {
	return strdup(_camera->getName().c_str());
}

bool	Camera_impl::hasFilterWheel() {
	return _camera->hasFilterWheel();
}

bool	Camera_impl::hasGuiderPort() {
	return _camera->hasGuiderPort();
}

} // namespace Astro
