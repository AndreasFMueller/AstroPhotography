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
	GuiderLocator	*locator;
public:
	GuiderFactoryI(astro::persistence::Database database,
		GuiderLocator *locator);
	virtual ~GuiderFactoryI();
	// conversions

	GuiderList	list(const Ice::Current& current);
	GuiderPrx	get(const GuiderDescriptor& descriptor,
				const Ice::Current& current);
	idlist	getAllCalibrations(const Ice::Current& current);
	idlist	getCalibrations(const GuiderDescriptor& guider,
				ControlType type,
				const Ice::Current& current);
	Calibration	getCalibration(int id,
				const Ice::Current& current);
	void	deleteCalibration(int id, const Ice::Current& current);
	int	addCalibration(const Calibration& calibration,
			const Ice::Current& current);
	idlist	getAllTracks(const Ice::Current& current);
	idlist	getTracks(const GuiderDescriptor& guider,
				const Ice::Current& current);
	TrackingHistory	getTrackingHistory(int id, const Ice::Current& current);
	TrackingHistory	getTrackingHistoryType(int id, ControlType type,
		const Ice::Current& current);
	TrackingSummary	getTrackingSummary(int id, const Ice::Current& current);
	TrackingSummary	getTrackingSummary(int id, ControlType type,
		const Ice::Current& current);
	void	deleteTrackingHistory(int id, const Ice::Current& current);
	astro::guiding::GuiderFactoryPtr	guiderfactory();
private:
	void	buildnewguider(const GuiderDescriptor& descriptor);
};

} // namespace snowstar

#endif /* _GuiderFactoryI_h */
