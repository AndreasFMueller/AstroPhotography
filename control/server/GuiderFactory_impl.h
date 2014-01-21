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

TrackingHistory	*getTrackingHistory(CORBA::Long id);
Calibration	*getCalibration(CORBA::Long id);

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
	virtual Astro::Guider_ptr	get(
		const Astro::GuiderDescriptor& descriptor);

	virtual Astro::GuiderFactory::idlist	*getCalibrations(
		const Astro::GuiderDescriptor& guider);
	virtual Astro::GuiderFactory::idlist	*getAllCalibrations();
	virtual Calibration	*getCalibration(CORBA::Long id);

	virtual Astro::GuiderFactory::idlist	*getGuideruns(
		const Astro::GuiderDescriptor& guider);
	virtual Astro::GuiderFactory::idlist	*getAllGuideruns();
	virtual TrackingHistory	*getTrackingHistory(CORBA::Long id);
};

} // namespace astro

#endif /* _GuiderFactory_impl_h */
