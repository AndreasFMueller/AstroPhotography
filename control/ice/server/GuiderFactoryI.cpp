/*
 * GuiderFactoryI.cpp -- guider factory servant implementation 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <GuiderFactoryI.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <GuiderI.h>
#include <ProxyCreator.h>
#include <IceConversions.h>
#include <AstroGuiding.h>
#include <CalibrationSource.h>
#include <AstroUtils.h>
#include <AstroEvent.h>
#include <AstroDevaccess.h>

namespace snowstar {

/**
 * \brief Create a GuiderFactory implementation object
 *
 * There will usually be only one instance of the factory. 
 * \param _locator		Locator for guiders, used to store a guider
 *				requested by a guider (the factory only
 *				returns a proxy, which will be converted to
 *				to an actual object by the locator)
 */
GuiderFactoryI::GuiderFactoryI(GuiderLocator *_locator) : locator(_locator) {
}

/**
 * \brief Destroy the guider factory object
 */
GuiderFactoryI::~GuiderFactoryI() {
}

/**
 * \brief Get a list of all the guiders available on the server
 *
 * This method forwards the request to the original guider factory and
 * converts the result retrieved so that the ICE interface understands it
 */
GuiderList	GuiderFactoryI::list(const Ice::Current& current) {
	CallStatistics::count(current);
	std::vector<astro::guiding::GuiderDescriptor>	l
		= guiderfactory()->list();
	GuiderList	result;
	std::vector<astro::guiding::GuiderDescriptor>::const_iterator	i;
	for (i = l.begin(); i != l.end(); i++) {
		result.push_back(convert(*i));
	}
	return result;
}

/**
 * \brief Get the proxy for a specific guider
 *
 * When a guider is created from the original guider factory, it must be
 * stored in the guider locator, whick keeps track of all guiders created
 * by this factory, and allows ICE to retrieve the guider when the client
 * tries to connect to it.
 *
 * \param descriptor	descriptor giving the name of the guider
 */
GuiderPrx	GuiderFactoryI::get(const GuiderDescriptor& descriptor,
			const Ice::Current& current) {
	CallStatistics::count(current);
	// name of the guider
	astro::guiding::GuiderDescriptor	d = convert(descriptor);
	std::string	gn = guiderdescriptor2name(descriptor);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request for guider named %s",
		gn.c_str());

	// if the locator does not have the guide port, we have to create it
	if (locator->has(gn)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "guider '%s' already exists",
			gn.c_str());
	} else {
		buildnewguider(descriptor);
	}

	// create a proxy
	std::string	ename = NameConverter::urlencode(gn);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "name for guider: %s", ename.c_str());
	GuiderPrx	guiderprx = createProxy<GuiderPrx>("guider/" + ename,
				current, false);
	if (guiderprx) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got a Guider proxy");
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "failed to construct GuiderPrx");
	}
	return guiderprx;
}

/**
 * \brief Build a new guider
 *
 * \param descriptor	the descriptor for the guider
 */
void	GuiderFactoryI::buildnewguider(const GuiderDescriptor& descriptor) {
	std::string	gn = guiderdescriptor2name(descriptor).c_str();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "building new guider for '%s'",
		gn.c_str());

	// get a GuiderPtr from the original factory
	astro::guiding::GuiderPtr	guider
		= guiderfactory()->get(convert(descriptor));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got the guider");

	// query the instrument for a guide rate
	astro::discover::InstrumentPtr	instrument
		= astro::discover::InstrumentBackend::get(
			descriptor.instrumentname);

	// find the local service name 
	std::string	localservice
		= astro::discover::ServiceLocation::get().servicename();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "local service: %s",
		localservice.c_str());

	// find out whether the insttrument has a Mount
	double	guiderate = -1;
	if (instrument->nComponentsOfType(
		astro::discover::InstrumentComponentKey::Mount) > 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "instrument has a mount");
		// get the mount
		astro::discover::InstrumentComponent    mount
			= instrument->getMount(0);
		std::string	mountname;
		if (localservice == mount.servicename()) {
			mountname = mount.deviceurl();
		} else {
			mountname = mount.remoteName().toString();
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found mount %s",
			mountname.c_str());

		try {
			// try to get the mount
			auto	repository
				= astro::module::ModuleRepository::get();
			astro::device::DeviceAccessor<astro::device::MountPtr>
					df(repository);
			astro::device::MountPtr	mountptr = df.get(mountname);

			// get the guide rate
			if (mountptr->hasGuideRates()) {
				astro::RaDec	guiderates
					= mountptr->getGuideRates();
				guiderate = guiderates.ra().degrees()
					/ (360. / 86400);
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"%s has guiderate %f",
					mountname.c_str(), guiderate);
			} else {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"%s does not have guiderates", 
					mountname.c_str());
			}
		} catch (const std::exception& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "failed to get guide "
				"rate from mount %s: %s", mountname.c_str(),
				x.what());
		}
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no mount components");
	}

	// get the guide from the instrument properties
	try {
		guiderate = instrument->getDouble("guiderate");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "guiderate property: %.3f",
			guiderate);
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no 'guiderate' property found");
	}

	// at this point we should have a reasonable guide rate
	if (guiderate > 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using guiderate %.2f",
			guiderate);
		guider->guiderate(guiderate);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "setting default guiderate %f",
			guider->guiderate());
	}

	// get the focallength from the instrument properties
	try {
		double	focallength
			= instrument->getDouble("guiderfocallength");
		guider->focallength(focallength);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "focallength: %.3f",
			focallength);
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no 'guiderfocallength' "
			"property found, using default %f",
			guider->focallength());
	}

	// create a GuiderI object
	Ice::ObjectPtr	guiderptr = new GuiderI(guider);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got the guiderptr");

	// add the guider we have constructed to the D
	locator->add(gn, guiderptr);
	astro::event(EVENT_CLASS, astro::events::INFO,
		astro::events::Event::GUIDE,
		astro::stringprintf("new guider: %s", gn.c_str()));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guider servant activated");
}

/**
 * \brief Get all calibrations stored in the database
 */
idlist	GuiderFactoryI::getAllCalibrations(const Ice::Current& current) {
	CallStatistics::count(current);
	astro::guiding::CalibrationStore	store;
	std::list<long> calibrations = store.getAllCalibrations();
	idlist	result;
	std::copy(calibrations.begin(), calibrations.end(),
		back_inserter(result));
	return result;
}

/**
 * \brief Get all the calibrations for a specific guider
 */
idlist	GuiderFactoryI::getCalibrations(const GuiderDescriptor& guider,
			ControlType type, const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get calibrations");
	astro::guiding::CalibrationStore	store;
	std::list<long> calibrations = store.getCalibrations(convert(guider),
						convertcontroltype(type));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d calibrations",
		calibrations.size());
	idlist	result;
	std::copy(calibrations.begin(), calibrations.end(),
		back_inserter(result));
	return result;
}

/**
 * \brief get details about a specific calibration
 */
Calibration	GuiderFactoryI::getCalibration(int id,
			const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve calibration %d", id);
	CalibrationSource	source;
	return source.get(id);
}

/**
 * \brief Delete a calibration
 */
void	GuiderFactoryI::deleteCalibration(int id,
			const Ice::Current& current) {
	CallStatistics::count(current);
	astro::guiding::CalibrationStore	store;
	if (!store.contains(id)) {
		NotFound	exception;
		exception.cause = astro::stringprintf("calibration %d not found", id);
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot delete: %s", exception.cause.c_str());
		throw exception;
	}
	store.deleteCalibration(id);
}

/**
 * \brief Add a calibration
 */
int	GuiderFactoryI::addCalibration(const Calibration& calibration,
			const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "storing a calibration");
	astro::guiding::CalibrationStore	store;
	// convert the calibration to a persistent calibration
	astro::guiding::CalibrationPtr	cal = convert(calibration);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new calibration: %s",
		cal->toString().c_str());
	astro::guiding::PersistentCalibration	pcal(*cal);

	// get the Instrument Backend
	std::string	instrumentname = calibration.guider.instrumentname;
	astro::discover::InstrumentBackend	instruments;
	if (!instruments.has(instrumentname)) {
		std::string	msg = astro::stringprintf("no instrument '%s'",
			instrumentname.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw BadState(msg);
	}
	astro::discover::InstrumentPtr	instrument
		= instruments.get(instrumentname);
	astro::discover::InstrumentComponent	ccdComponent
		= instrument->getGuiderCcd(0);

	pcal.instrument = instrumentname;
	pcal.ccd = ccdComponent.deviceurl();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found ccd: %s", pcal.ccd.c_str());
	switch (calibration.type) {
	case snowstar::ControlGuidePort:
		{
		astro::discover::InstrumentComponent	guideportComponent
			= instrument->getGuidePort(0);
		pcal.controldevice = guideportComponent.deviceurl();
		}
		break;
	case snowstar::ControlAdaptiveOptics:
		{
		astro::discover::InstrumentComponent	adaptiveopticsComponent
			= instrument->getAdaptiveOptics(0);
		pcal.controldevice = adaptiveopticsComponent.deviceurl();
		}
		break;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found control device: %s",
		pcal.controldevice.c_str());

	// parsing the name
	astro::guiding::GuiderName	guidername(instrumentname);
	pcal.instrument = guidername.instrument();

	// convert the calibration name
	debug(LOG_DEBUG, DEBUG_LOG, 0, "instrument=%s", pcal.instrument.c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd=%s", pcal.ccd.c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "controldevice=%s", pcal.controldevice.c_str());
	long	id = store.addCalibration(pcal);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration stored as %ld", id);
	return id;
}

/**
 * \brief Get all guide run ids available in the database
 */
idlist	GuiderFactoryI::getAllTracks(const Ice::Current& current) {
	CallStatistics::count(current);
	astro::guiding::TrackingStore	store;
	std::list<long>	trackings = store.getAllTrackings();
	idlist	result;
	std::copy(trackings.begin(), trackings.end(), back_inserter(result));
	return result;
}

/**
 * \brief Get the guide run ids for a specific guider
 */
idlist	GuiderFactoryI::getTracks(const GuiderDescriptor& guider,
			const Ice::Current& current) {
	CallStatistics::count(current);
	astro::guiding::TrackingStore	store;
	std::list<long>	trackings = store.getTrackings(convert(guider));
	idlist	result;
	std::copy(trackings.begin(), trackings.end(), back_inserter(result));
	return result;
}

/**
 * \brief Get the tracking history of a specific guide run
 *
 * Note that the result of this operation can be large. A guide run of an
 * hour with 5 updates per second (using an adaptive optics unit) contains
 * 18000 data points. Normal guide runs with 10 second update intervals
 * are quite manageable in size, about 360 points per hour of guiding.
 */
TrackingHistory	GuiderFactoryI::getTrackingHistory(int id,
			const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve history %d", id);
	try {
		astro::guiding::TrackingStore	store;
		TrackingHistory	history = convert(store.get(id));
		return history;
	} catch (const astro::persistence::NotFound& ex) {
		std::string	msg = astro::stringprintf("tracking history %d "
			"not found: %s", id, ex.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw NotFound(msg);
	} catch (const std::exception& ex) {
		std::string	cause = astro::stringprintf(
			"tracking history %d not found: %s (%s)",
			astro::demangle_string(ex).c_str(), ex.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s");
		throw NotFound(cause);
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no history (unknown reason)");
		throw NotFound("(unknown reason)");
	}
	throw std::runtime_error("internal error");
}

/**
 * \brief Get a tracking history by id and type
 *
 * The tracking history usually contains tracking points measured by the
 * AO unit as well as the guider port. By specifying the type, we select
 * only the tracking points of that particular type.
 */
TrackingHistory	GuiderFactoryI::getTrackingHistoryType(int id,
	ControlType type, const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve history %d", id);
	try {
		astro::guiding::TrackingStore	store;
		switch (type) {
		case ControlGuidePort: {
			TrackingHistory	history = convert(store.get(id,
				astro::guiding::GP));
			return history;
			}
		case ControlAdaptiveOptics: {
			TrackingHistory	history = convert(store.get(id,
				astro::guiding::AO));
			return history;
			}
		}
	} catch (const astro::persistence::NotFound& ex) {
		std::string	msg = astro::stringprintf("tracking history %d "
			"not found: %s", id, ex.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw NotFound(msg);
	} catch (const std::exception& ex) {
		std::string	cause
			= astro::stringprintf("no history: %s(%s)",
				astro::demangle_string(ex).c_str(), ex.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw NotFound(cause);
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no history (unknown reason)");
		throw NotFound("(unknown reason)");
	}
	throw std::runtime_error("internal error");
}

/**
 * \brief Get a summary of the track
 *
 * The tracking history contains all the tracking points, which is often
 * way too much information. This 
 */
TrackingSummary	GuiderFactoryI::getTrackingSummary(int id,
			const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		astro::guiding::TrackingStore	store;
		return convert(store.getSummary(id));
	} catch (const astro::persistence::NotFound& ex) {
		std::string	msg = astro::stringprintf(
			"track %d not found: %s", id, ex.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw NotFound(msg);
	} catch (const std::exception& ex) {
		std::string	cause = astro::stringprintf(
			"no summary %d: %s(%s)", id, 
			astro::demangle_string(ex).c_str(), ex.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw NotFound(cause);
	} catch (...) {
		throw NotFound("unknown reason");
	}
	throw std::runtime_error("not implemented yet");
}

/**
 * \brief Delete a tracking history from the database
 */
void	GuiderFactoryI::deleteTrackingHistory(int id,
		const Ice::Current& current) {
	CallStatistics::count(current);
	astro::guiding::TrackingStore	store;
	if (!store.contains(id)) {
		NotFound	exception;
		exception.cause = astro::stringprintf("tracking history %d not "
			"found", id);
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot delete: %s",
			exception.cause.c_str());
		throw exception;
	}
	store.deleteTrackingHistory(id);
}

/**
 * \brief Get the guider factory
 */
astro::guiding::GuiderFactoryPtr	GuiderFactoryI::guiderfactory() {
	return astro::guiding::GuiderFactory::get();
}

} // namespace snowstar
