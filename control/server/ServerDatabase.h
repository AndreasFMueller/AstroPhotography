/*
 * ServerDatabase.h -- Collection 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ServerDatabase_h
#define _ServerDatabase_h

#include <AstroPersistence.h>
#include <guider.hh>

namespace Astro {

class ServerDatabase {
public:
	ServerDatabase(const std::string& filename);
	ServerDatabase();
	astro::persistence::Database	database();
	TrackingHistory	*getTrackingHistory(CORBA::Long id);
	Calibration	*getCalibration(CORBA::Long id);
};

} // namespace Astro

#endif /* _ServerDatabase_h */
