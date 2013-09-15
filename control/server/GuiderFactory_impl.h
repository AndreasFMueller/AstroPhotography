/*
 * GuiderFactory_impl.h -- GuiderFactory servant implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _GuiderFactory_impl_h
#define _GuiderFactory_impl_h

#include <guider.hh>
#include <AstroGuiding.h>

namespace Astro {

/**
 * \brief GuiderFactory servant definition
 */
class GuiderFactory_impl : public POA_Astro::GuiderFactory {
	astro::guiding::GuiderFactoryPtr	_guiderfactory;
public:
	inline GuiderFactory_impl(astro::guiding::GuiderFactoryPtr guiderfactory)
		: _guiderfactory(guiderfactory) { }
	virtual ~GuiderFactory_impl() { }
	virtual Astro::GuiderFactory::GuiderList	*list();
	virtual Astro::Guider_ptr	get(const Astro::GuiderFactory::GuiderDescriptor& descriptor);
};

} // namespace astro

#endif /* _GuiderFactory_impl_h */
