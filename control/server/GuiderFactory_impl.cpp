/*
 * GuiderFactory_impl.cpp -- implementation of the GuiderFactory servant
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <GuiderFactory_impl.h>
#include <Guider_impl.h>
#include <Conversions.h>

namespace Astro {

//////////////////////////////////////////////////////////////////////
// GuiderFactory implementation
//////////////////////////////////////////////////////////////////////

GuiderFactory::GuiderList	*GuiderFactory_impl::list() {
	std::vector<astro::guiding::GuiderDescriptor>	l = _guiderfactory->list();
	return NULL;
}

Guider_ptr	GuiderFactory_impl::get(
	const Astro::GuiderFactory::GuiderDescriptor& descriptor) {
	astro::guiding::GuiderPtr	guider
		= _guiderfactory->get(astro::convert(descriptor));
	Astro::Guider_impl	*g = new Astro::Guider_impl(guider);
	return g->_this();
}

} // namespace Astro
