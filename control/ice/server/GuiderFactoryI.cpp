/*
 * GuiderFactoryI.cpp -- guider factory servant implementation 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <GuiderFactoryI.h>
#include <CalibrationStore.h>
#include <TrackingStore.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <GuiderI.h>
#include <ProxyCreator.h>
#include <IceConversions.h>

namespace snowstar {

GuiderFactoryI::GuiderFactoryI(astro::persistence::Database _database,
		astro::guiding::GuiderFactory& _guiderfactory,
		GuiderLocator *_locator,
		ImageDirectory& _imagedirectory)
	: database(_database), guiderfactory(_guiderfactory),
	  locator(_locator), imagedirectory(_imagedirectory) {
}

GuiderFactoryI::~GuiderFactoryI() {
}

GuiderList	GuiderFactoryI::list(const Ice::Current& /* current */) {
	std::vector<astro::guiding::GuiderDescriptor>	l
		= guiderfactory.list();
	GuiderList	result;
	std::vector<astro::guiding::GuiderDescriptor>::const_iterator	i;
	for (i = l.begin(); i != l.end(); i++) {
		result.push_back(convert(*i));
	}
	return result;
}

/**
 * \brief Get the proxy
 */
GuiderPrx	GuiderFactoryI::get(const GuiderDescriptor& descriptor,
			const Ice::Current& current) {
	// name of the guider
	astro::guiding::GuiderDescriptor	d = convert(descriptor);
	std::string	guidername = d.toString();

	// get an GuiderPtr from teh 
	astro::guiding::GuiderPtr	guider = guiderfactory.get(d);

	// create a GuiderI object
	Ice::ObjectPtr	guiderptr = new GuiderI(guider, imagedirectory,
		database);

	// add the guider we have constructed to the D
	locator->add(guidername, guiderptr);

	// create a proxy
	std::string	ename = NameConverter::urlencode(guidername);
	return createProxy<GuiderPrx>("guider/" + ename, current, false);
}

idlist	GuiderFactoryI::getAllCalibrations(const Ice::Current& /* current */) {
	astro::guiding::CalibrationStore	store(database);
	std::list<long> calibrations = store.getAllCalibrations();
	idlist	result;
	std::copy(calibrations.begin(), calibrations.end(),
		back_inserter(result));
	return result;
}

idlist	GuiderFactoryI::getCalibrations(const GuiderDescriptor& guider,
			const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get calibrations");
	astro::guiding::CalibrationStore	store(database);
	std::list<long> calibrations = store.getCalibrations(convert(guider));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d calibrations",
		calibrations.size());
	idlist	result;
	std::copy(calibrations.begin(), calibrations.end(),
		back_inserter(result));
	return result;
}

Calibration	GuiderFactoryI::getCalibration(int id,
			const Ice::Current& /* current */) {
	// use the database to retrieve the complete calibration data
	Calibration	calibration;
	try {
		astro::guiding::CalibrationTable	ct(database);
		astro::guiding::CalibrationRecord	r = ct.byid(id);
		calibration.id = r.id();
		calibration.timeago = converttime(r.when);
		calibration.guider.cameraname = r.camera;
		calibration.guider.ccdid = r.ccdid;
		calibration.guider.guiderportname = r.guiderport;
		for (int i = 0; i < 6; i++) {
			calibration.coefficients.push_back(r.a[i]);
		}

		// add calibration points
		astro::guiding::CalibrationStore	store(database);
		std::list<astro::guiding::CalibrationPointRecord>	points
			= store.getCalibrationPoints(id);
		std::list<astro::guiding::CalibrationPointRecord>::iterator i;
		for (i = points.begin(); i != points.end(); i++) {
			calibration.points.push_back(convert(*i));
		}
		return calibration;
	} catch (std::exception& ex) {
		std::string	msg = astro::stringprintf("calibrationd run %d "
			"not found: %s", id, ex.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw NotFound(msg);
	}
}

idlist	GuiderFactoryI::getAllGuideruns(const Ice::Current& /* current */) {
	astro::guiding::TrackingStore	store(database);
	std::list<long>	trackings = store.getAllTrackings();
	idlist	result;
	std::copy(trackings.begin(), trackings.end(), back_inserter(result));
	return result;
}

idlist	GuiderFactoryI::getGuideruns(const GuiderDescriptor& guider,
			const Ice::Current& /* current */) {
	astro::guiding::TrackingStore	store(database);
	std::list<long>	trackings = store.getTrackings(convert(guider));
	idlist	result;
	std::copy(trackings.begin(), trackings.end(), back_inserter(result));
	return result;
}

TrackingHistory	GuiderFactoryI::getTrackingHistory(int id,
			const Ice::Current& /* current */) {
	astro::guiding::TrackingStore	store(database);
	TrackingHistory	history;
	history.guiderunid = id;

	// get other attributes
	try {
		astro::guiding::GuidingRunTable	gt(database);
		astro::guiding::GuidingRunRecord	r = gt.byid(id);
		history.timeago = converttime(r.whenstarted);
		history.guider.cameraname = r.camera;
		history.guider.ccdid = r.ccdid;
		history.guider.guiderportname = r.guiderport;

		// tracking points
		astro::guiding::TrackingStore	store(database);
		std::list<astro::guiding::TrackingPointRecord>	points
			= store.getHistory(id);
		std::list<astro::guiding::TrackingPointRecord>::iterator i;
		for (i = points.begin(); i != points.end(); i++) {
			history.points.push_back(convert(*i));
		}

		// done
		return history;
	} catch (const std::exception& ex) {
		std::string	msg = astro::stringprintf("tracking history %d "
			"not found: %s", id, ex.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw NotFound(msg);
	}
}

} // namespace snowstar
