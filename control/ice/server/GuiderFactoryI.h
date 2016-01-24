/*
 * GuiderFactoryI.h -- guider factory interface declarations
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _GuiderFactoryI_h
#define _GuiderFactoryI_h

#include <guider.h>
#include <AstroGuiding.h>
#include <GuiderLocator.h>
#include <ImageDirectory.h>

namespace snowstar {

/**
 * \brief The guider factory implementation object
 *
 * This class implements the servant for the guider factory interface.
 * It is essentially a wrapper around the original guiderfactory object,
 * but it needs some support structure, e.g. the database for persistence.
 */
class GuiderFactoryI : virtual public GuiderFactory {
	astro::persistence::Database	database;
	astro::guiding::GuiderFactory&	guiderfactory;
	GuiderLocator	*locator;
	astro::image::ImageDirectory	imagedirectory;
public:
	GuiderFactoryI(astro::persistence::Database database,
		astro::guiding::GuiderFactory& guiderfactory,
		GuiderLocator *locator,
		astro::image::ImageDirectory& imagedirectory);
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
	void	deleteCalibration(int id, const Ice::Current& current);
	idlist	getAllGuideruns(const Ice::Current& current);
	idlist	getGuideruns(const GuiderDescriptor& guider,
				const Ice::Current& current);
	TrackingHistory	getTrackingHistory(int id, const Ice::Current& current);
	TrackingHistory	getTrackingHistoryType(int id, CalibrationType type,
		const Ice::Current& current);
	void	deleteTrackingHistory(int id, const Ice::Current& current);
};

} // namespace snowstar

#endif /* _GuiderFactoryI_h */
