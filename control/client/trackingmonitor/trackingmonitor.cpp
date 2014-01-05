/*
 * trackingmonitor.cpp -- demo program for the tracking monitor functionality
 *                        of the astrod server
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperwil
 */
#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <OrbSingleton.h>
#include <NameService.h>
#include <AstroDebug.h>
#include <includes.h>
#include <CorbaExceptionReporter.h>
#include <guider.hh>
#include <signal.h>
#include <iomanip>
#include <AstroUtils.h>

namespace astro {

//////////////////////////////////////////////////////////////////////
// TrackingMonitor service implementation
//////////////////////////////////////////////////////////////////////
std::ostream&	operator<<(std::ostream& out, const Astro::Point& p) {
	out << std::setw(7) << std::fixed << std::setprecision(3) << p.x;
	out << ",";
	out << std::setw(7) << std::fixed << std::setprecision(3) << p.y;
	return out;
}

std::ostream&	operator<<(std::ostream& out, const ::Astro::TrackingInfo& ti) {
	out << std::fixed << std::setprecision(3) << Timer::gettime() - ti.timeago;
	out << "     ";
	out << ti.trackingoffset;
	out << "     ";
	out << ti.activation;
	return out;
}

class TrackingMonitor_impl : public POA_Astro::TrackingMonitor {
public:
	TrackingMonitor_impl() { }
	virtual void	update(const ::Astro::TrackingInfo& ti);
};

void	TrackingMonitor_impl::update(const ::Astro::TrackingInfo& ti) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "update() received");
	std::cout << ti << std::endl;
}

//////////////////////////////////////////////////////////////////////
// TrackingImageMonitor service implementation
//////////////////////////////////////////////////////////////////////
class TrackingImageMonitor_impl : public POA_Astro::TrackingImageMonitor {
public:
	TrackingImageMonitor_impl() { }
	virtual void	update(const ::Astro::ImageSize& size,
		const ::Astro::ShortSequence& imagedata);
};

void	TrackingImageMonitor_impl::update(const ::Astro::ImageSize& size,
		const ::Astro::ShortSequence& imagedata) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got an image of size %dx%d",
		size.width, size.height);
	double	min = 65536;
	double	max = 0;
	double	mean = 0;
	for (unsigned int i = 0; i < imagedata.length(); i++) {
		unsigned short	v = imagedata[i];
		if (v > max) { max = v; }
		if (v < min) { min = v; }
		mean += v;
	}
	mean /= imagedata.length();
	std::cout << size.width << "x" << size.height << " image, ";
	std::cout << "min=";
	std::cout << std::fixed << std::setprecision(0) << min;
	std::cout << ", ";
	std::cout << "mean=";
	std::cout << std::fixed << std::setprecision(1) << mean;
	std::cout << ", ";
	std::cout << "max=";
	std::cout << std::fixed << std::setprecision(0) << max;
	std::cout << std::endl;
}

Astro::Guider_var	guider;
long	monitorid = 0;
long	imagemonitorid = 0;
CORBA::ORB_ptr	orbptr;

void	signal_handler(int sig) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "signal %d received", sig);
	guider->unregisterMonitor(monitorid);
	guider->unregisterImageMonitor(imagemonitorid);
	orbptr->shutdown(false);
}

int	trackingmonitor_main(int argc, char *argv[]) {

	// initialize the ORB
	Astro::OrbSingleton	orb(argc, argv);

	// guider parameters
	std::string	camera("camera:simulator/camera");
	int	ccdid = 0;
	std::string	guiderport("guiderport:simulator/guiderport");

	// parse the commmand line for guider information
	int	c;
	while (EOF != (c = getopt(argc, argv, "dC:c:g:")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'C':
			camera = optarg;
			break;
		case 'c':
			ccdid = atoi(optarg);
			break;
		case 'g':
			guiderport = optarg;
			break;
		}

	// access the naming service
	Astro::Naming::NameService	nameservice(orb);

	// get a reference to the guider factory
	Astro::GuiderFactory_var	guiderfactory;
	try {
		guiderfactory = orb.getGuiderfactory();
	} catch (const CORBA::Exception& x) {
		std::string	s = Astro::exception2string(x);
		debug(LOG_ERR, DEBUG_LOG, 0, "getGuiderfactory() exception: %s",
			s.c_str());
		throw std::runtime_error(s);
	}

	// get a guider from the guider factory
	Astro::GuiderDescriptor	*descriptor = new Astro::GuiderDescriptor();
	descriptor->cameraname = CORBA::string_dup(camera.c_str());
	descriptor->ccdid = ccdid;
	descriptor->guiderportname = CORBA::string_dup(guiderport.c_str());
	guider = guiderfactory->get(*descriptor);

	// create a POA for the local tracking monitor implementation
	CORBA::Object_var	obj
		= orb.orbvar()->resolve_initial_references("RootPOA");
	PortableServer::POA_var	root_poa = PortableServer::POA::_narrow(obj);
	assert(!CORBA::is_nil(root_poa));

	// create a TrackingMonintor implementation and hand it to the POA
	TrackingMonitor_impl	*trackingmonitor = new TrackingMonitor_impl();
	PortableServer::ObjectId_var	trackingmonitorsid
		= root_poa->activate_object(trackingmonitor);

	// now get a reference to object ourselves
	CORBA::Object_var	tmobj
		= root_poa->id_to_reference(trackingmonitorsid);
	Astro::TrackingMonitor_var	tmvar
		= Astro::TrackingMonitor::_narrow(tmobj);

	// create the TrackingImageMonitor implementation and hand it to the POA
	TrackingImageMonitor_impl	*trackingimagemonitor
		= new TrackingImageMonitor_impl();
	PortableServer::ObjectId_var	trackingimagemonitorsid
		= root_poa->activate_object(trackingimagemonitor);

	// now get a reference to the object
	CORBA::Object_var	timobj
		= root_poa->id_to_reference(trackingimagemonitorsid);
	Astro::TrackingImageMonitor_var	timvar
		= Astro::TrackingImageMonitor::_narrow(timobj);

	// get the POA manager
	PortableServer::POAManager_var	pman = root_poa->the_POAManager();
	pman->activate();

	// register the tracking monitor with the guider
	monitorid = guider->registerMonitor(tmvar);
	imagemonitorid = guider->registerImageMonitor(timvar);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "monitor registered as %ld", monitorid);

	// make the orb ptr available to the signal handler
	orbptr = orb.orbvar();

	// register signal handler
	signal(SIGINT, signal_handler);

	// wait for requests coming into the orb
	orb.orbvar()->run();
	orb.orbvar()->destroy();

	// if we get to this point, then the orb was interrupted by the
	// signal handler
	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::trackingmonitor_main(argc, argv);
	} catch (std::exception& x) {
		std::cout << argv[0] << " terminated by exception: ";
		std::cout << x.what() << std::endl;
	}
}
