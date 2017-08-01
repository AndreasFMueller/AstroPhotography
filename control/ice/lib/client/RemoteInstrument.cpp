/*
 * RemoteInstrument.cpp -- class to access remote interfaces
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <RemoteInstrument.h>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <AstroFormat.h>
#include <CommunicatorSingleton.h>
#include <IceConversions.h>
#include <Ice/Ice.h>

using namespace astro::persistence;
using namespace astro::config;

namespace snowstar {

/**
 * \brief Construct a remote instrument
 */
RemoteInstrument::RemoteInstrument(InstrumentsPrx instruments,
	const std::string& name) : _name(name) {
	// make sure that there is such an instrument
	if (!instruments->has(name)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no instrument '%s'",
			name.c_str());
		throw std::runtime_error("no such instrument");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "instrument %s exists", _name.c_str());
	_instrument = instruments->get(name);
}

/**
 * \brief uninitialized remote instrument
 */
RemoteInstrument::RemoteInstrument() {
}

RemoteInstrument::RemoteInstrument(const RemoteInstrument& other)
	: _instrument(other._instrument), _name(other._name) {
}

RemoteInstrument&	RemoteInstrument::operator=(const RemoteInstrument& other) {
	_instrument = other._instrument;
	_name = other._name;
	return *this;
}

/**
 * \brief get the number of components of a given type
 */
unsigned int	RemoteInstrument::componentCount(InstrumentComponentType type) {
	return _instrument->nComponentsOfType(type);
}

bool	RemoteInstrument::has(InstrumentComponentType type, unsigned int index) {
	return (componentCount(type) > index) ? true : false;
}

/**
 * \brief Get Component of a given type and index
 */
InstrumentComponent	RemoteInstrument::getComponent(
			InstrumentComponentType type, unsigned int index) {
	// check whether the component exists
	if (!has(type, index)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "index %u too large", index);
		throw std::runtime_error("no such component");
	}
	// now get the component
	InstrumentComponent	component
		= _instrument->getComponent(type, index);
	return component;
}

/**
 * \brief Retrieve the servername for a component of a given type and index
 */
astro::ServerName	RemoteInstrument::servername(
				InstrumentComponentType type,
				unsigned int index) {
	InstrumentComponent	component = getComponent(type, index);

	// get the AO device for mapped or direct components
	return astro::ServerName(component.servicename);
}

/**
 * \brief Retrieve a Devices proxy for a given server name
 */
DevicesPrx	RemoteInstrument::devices(const astro::ServerName& servername) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve remote devices from %s",
		servername.host().c_str());

	// connect to the remote server
	std::string	connectstring
		= astro::stringprintf("Devices:default -h %s -p %hu",
			servername.host().c_str(), servername.port());

	// get a communicator ptr (singleton?)
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();

	// get the Devices object
	Ice::ObjectPrx	base = ic->stringToProxy(connectstring);
	DevicesPrx	devices = DevicesPrx::checkedCast(base);
	return devices;
}

/**
 * \brief Retrieve adaptive optics proxy for an instrument
 */
AdaptiveOpticsPrx       RemoteInstrument::adaptiveoptics(unsigned int index) {
	InstrumentComponent	component
			= getComponent(InstrumentAdaptiveOptics, index);

	// get the AO device for mapped or direct components
	return devices(astro::ServerName(component.servicename))
			->getAdaptiveOptics(component.deviceurl);
}

/**
 * \brief Retrieve a camera proxy
 */
CameraPrx               RemoteInstrument::camera(unsigned int index) {
	InstrumentComponent	component
			= getComponent(InstrumentCamera, index);

	// get the AO device for mapped or direct components
	return devices(astro::ServerName(component.servicename))
			->getCamera(component.deviceurl);
}

/**
 * \brief Retrieve a ccd proxy
 */
CcdPrx                  RemoteInstrument::ccd(unsigned int index) {
	InstrumentComponent	component
			= getComponent(InstrumentCCD, index);

	// get the AO device for mapped or direct components
	return devices(astro::ServerName(component.servicename))
			->getCcd(component.deviceurl);
}

/**
 * \brief Retrieve a cooler proxy
 *
 * Coolers can be derived from a ccd
 */
CoolerPrx               RemoteInstrument::cooler(unsigned int index) {
	InstrumentComponent	component
			= getComponent(InstrumentCooler, index);

	// get the AO device for mapped or direct components
	return devices(astro::ServerName(component.servicename))
			->getCooler(component.deviceurl);
}

/**
 * \brief Retrieve  Filterwheel proxy
 *
 * Filterwheels can be derived from a camera
 */
FilterWheelPrx          RemoteInstrument::filterwheel(unsigned int index) {
	InstrumentComponent	component
			= getComponent(InstrumentFilterWheel, index);

	// get the AO device for mapped or direct components
	return devices(astro::ServerName(component.servicename))
			->getFilterWheel(component.deviceurl);
}

/**
 * \brief Retrieve a Focuser proxy
 */
FocuserPrx              RemoteInstrument::focuser(unsigned int index) {
	InstrumentComponent	component
			= getComponent(InstrumentFocuser, index);

	// get the AO device for mapped or direct components
	return devices(astro::ServerName(component.servicename))
			->getFocuser(component.deviceurl);
}

/**
 * \brief Retrive a guider ccd proxy
 *
 * guider ports can be derived from a camera
 */
CcdPrx           RemoteInstrument::guiderccd(unsigned int index) {
	InstrumentComponent	component
			= getComponent(InstrumentGuiderCCD, index);

	// get the AO device for mapped or direct components
	return devices(astro::ServerName(component.servicename))
			->getCcd(component.deviceurl);
}

/**
 * \brief Retrive a guider port proxy
 *
 * guider ports can be derived from a camera
 */
GuidePortPrx           RemoteInstrument::guideport(unsigned int index) {
	InstrumentComponent	component
			= getComponent(InstrumentGuidePort, index);

	// get the AO device for mapped or direct components
	return devices(astro::ServerName(component.servicename))
			->getGuidePort(component.deviceurl);
}

/**
 * \brief Retrieve a mount proxy
 */
MountPrx                RemoteInstrument::mount(unsigned int index) {
	InstrumentComponent	component
			= getComponent(InstrumentMount, index);

	// get the AO device for mapped or direct components
	return devices(astro::ServerName(component.servicename))
			->getMount(component.deviceurl);
}

/**
 * \brief Retrieve a Guider
 */
GuiderPrx	RemoteInstrument::guider(unsigned int ccdindex,
			unsigned int guideportindex, unsigned int aoindex) {
	// make sure we have enough 
	if (ccdindex >= _instrument->nComponentsOfType(InstrumentGuiderCCD)) {
		throw std::runtime_error("now guider CCD found");
	}

	// we ask for the component of the GuiderCCD, because that is
	// where the guider will reside
	InstrumentComponent	component
			= getComponent(InstrumentGuiderCCD, ccdindex);
	astro::ServerName	servername(component.servicename);

	// make sure we can get all those devices

	// get a communicator ptr (singleton?)
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	Ice::ObjectPrx	gbase
		= ic->stringToProxy(servername.connect("Guiders"));
	GuiderFactoryPrx	guiderfactory
		= GuiderFactoryPrx::checkedCast(gbase);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a guider factory");

	// now build the descriptor
	GuiderDescriptor	guiderdescriptor;
	guiderdescriptor.instrumentname = name();
	guiderdescriptor.ccdIndex = ccdindex;

	// check guideport
	guiderdescriptor.guideportIndex = guideportindex;
	int	n = _instrument->nComponentsOfType(InstrumentGuidePort);
	if (guideportindex >= n) {
		guiderdescriptor.guideportIndex = -1;
	}

	// check adaptive optics
	guiderdescriptor.adaptiveopticsIndex = aoindex;
	n = _instrument->nComponentsOfType(InstrumentAdaptiveOptics);
	if (aoindex >= n) {
		guiderdescriptor.adaptiveopticsIndex = -1;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getting '%s|%d|%d|%d'",
		guiderdescriptor.instrumentname.c_str(),
		guiderdescriptor.ccdIndex,
		guiderdescriptor.guideportIndex,
		guiderdescriptor.adaptiveopticsIndex);

	// retrieve the guider factory
	GuiderPrx	guider = guiderfactory->get(guiderdescriptor);
	if (guider) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got guider with ccd %s",
			convert(guider->getCcd()->getInfo()).toString().c_str());
	}
	return guider;
}

} // namespace snowstar
