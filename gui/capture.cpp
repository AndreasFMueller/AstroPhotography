/*
 * capture_main.cpp -- small image capture application
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <QApplication>
#include <AstroLoader.h>
#include <AstroDevice.h>
#include <AstroCamera.h>
#include <string.h>
#include <AstroDebug.h>
#include <capturewindow.h>

using namespace astro::module;
using namespace astro::camera;
using namespace astro::device;

namespace astro {

int	main(int argc, char *argv[]) {
	const char	*modulename = "uvc";
	int	cameraid = 0;
	int	ccdid = 0;
	int	c;
	
	while (EOF != (c = getopt(argc, argv, "dC:c:m:")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'C':
			cameraid = atoi(optarg);
			break;
		case 'c':
			ccdid = atoi(optarg);
			break;
		case 'm':
			modulename = strdup(optarg);
			break;
		}

	// open repository
	Repository	repository;
	ModulePtr	module = repository.getModule(modulename);
	module->open();

	// Device locator for the module
	DeviceLocatorPtr	locator = module->getDeviceLocator();

	// get cameras
	std::vector<std::string>	cameras = locator->getDevicelist();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d devices", cameras.size());
	if (cameraid >= cameras.size()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "not enough devices");
		std::runtime_error("camera id too large");
	}
	CameraPtr	camera = locator->getCamera(cameras[cameraid]);
	CcdPtr	ccd = camera->getCcd(ccdid);
	
	// now initialize the GUI stuff
        QApplication    app(argc, argv);
	CaptureWindow	capturewindow;
	capturewindow.setCamera(camera);
	capturewindow.setCcd(ccd);
	capturewindow.show();
	app.installEventFilter(&capturewindow);
        return app.exec();
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main(argc, argv);
	} catch (std::exception& x) {
		std::cerr << "capture terminated by exception:" << x.what()
			<< std::endl;
	}
}
