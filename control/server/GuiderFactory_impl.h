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
public:
	inline GuiderFactory_impl() { }
	virtual ~GuiderFactory_impl() { }
	virtual Astro::GuiderFactory::GuiderList	*list();
	virtual Astro::_objref_Guider	*get(const Astro::GuiderFactory::GuiderDescriptor& descriptor);
};

} // namespace astro

#endif /* _GuiderFactory_impl_h */
