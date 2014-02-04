/*
 * GuiderFactoryI.h -- guider factory interface declarations
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _GuiderFactoryI_h
#define _GuiderFactoryI_h

#include <guider.h>
#include <AstroGuiding.h>

namespace snowstar {

GuiderDescriptor	convert(const astro::guiding::GuiderDescriptor& gd);
astro::guiding::GuiderDescriptor	convert(const GuiderDescriptor& gd);

TrackingPoint	convert(const astro::guiding::TrackingPoint& trackingpoint);

CalibrationPoint	convert(const astro::guiding::CalibrationPoint& cp);

class GuiderFactoryI : public GuiderFactory {
	astro::persistence::Database	database;
	astro::guiding::GuiderFactoryPtr	guiderfactory;
public:
	GuiderFactoryI(astro::persistence::Database database,
		astro::guiding::GuiderFactoryPtr guiderfactory);
	virtual ~GuiderFactoryI();
	// conversions

	GuiderList	list(const Ice::Current& current);
	GuiderPrx	get(const GuiderDescriptor& descriptor,
				const Ice::Current& current);
	idlist	getAllCalibrations(const Ice::Current& current);
	idlist	getCalibrations(const GuiderDescriptor& guider,
				const Ice::Current& current);
	Calibration	getCalibration(int id,
				const Ice::Current& current);
	idlist	getAllGuideruns(const Ice::Current& current);
	idlist	getGuideruns(const GuiderDescriptor& guider,
				const Ice::Current& current);
	TrackingHistory	getTrackingHistory(int id, const Ice::Current& current);
};

} // namespace snowstar

#endif /* _GuiderFactoryI_h */
