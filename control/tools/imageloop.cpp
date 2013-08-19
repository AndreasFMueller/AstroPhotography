/*
 * imageloop.cpp -- program to retrieve images from a camera in a loop
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <stdexcept>
#include <AstroDebug.h>
#include <AstroLoader.h>
#include <AstroCamera.h>
#include <AstroIO.h>
#include <AstroFormat.h>
#include <AstroFilter.h>
#include <AstroFilterfunc.h>
#include <AstroDevice.h>
#include <AstroLoop.h>
#include <AstroCallback.h>
#include <Sun.h>

using namespace astro;
using namespace astro::io;
using namespace astro::module;
using namespace astro::camera;
using namespace astro::device;
using namespace astro::image::filter;
using namespace astro::task;
using namespace astro::callback;

namespace astro {

/**
 * \brief Usage of the imageloop program
 */
void	usage(const char *progname) {
	std::cout << "usage: " << progname << " [ -adNF? ] [ -m module ] [ -C cameraid ] [ -c ccdid ] [ -n nimages ] [ -p period ] [ -E targetmean ] [ -e exposuretime ] [ -w width ] [ -h height ] [ -x xoffset ] [ -y yoffset ] [ -L longitude ] [ -l latitude ] [ -o directory ]" << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -d           increase deug level" << std::endl;
	std::cout << "  -m module    load camera module" << std::endl;
	std::cout << "  -C cameraid  which camera to use, default 0" << std::endl;
	std::cout << "  -c ccdid     which ccd to use, default 0" << std::endl;
	std::cout << "  -n nimages   number of images to retrieve" << std::endl;
	std::cout << "  -p period    image period" << std::endl;
	std::cout << "  -w width     width of image rectangle" << std::endl;
	std::cout << "  -h height    height of image rectangle" << std::endl;
	std::cout << "  -x xoffset   horizontal offset of image rectangle"
		<< std::endl;
	std::cout << "  -y yoffset   vertical offset of image rectangle"
		<< std::endl;
	std::cout << "  -L longitude logitude of the camera location"
		<< std::endl;
	std::cout << "  -l latitude  latitude of the camera location"
		<< std::endl;
	std::cout << "  -N           take images during the night only"
		<< std::endl;
	std::cout << "  -n images    number of images, 0 means never stop"
		<< std::endl;
	std::cout << "  -o outdir    directory where files should be placed"
		<< std::endl;
	std::cout << "  -t           use timestamps as filenames" << std::endl;
	std::cout << "  -e time      (initial) exposure time, modified later if target mean set" << std::endl;
	std::cout << "  -E mean      attempt to vary the exposure time in such a way that" << std::endl;
	std::cout << "               that the mean pixel value stays close to <mean>" << std::endl;
	std::cout << "  -M meadian   attemtp to vary the exposure time in such a way that" << std::endl;
	std::cout << "               that the median pixel value stays close to the <median>" << std::endl;
	std::cout << "  -F           stay in the foreground" << std::endl;
	std::cout << "  -P prog      processing script for individual images" << std::endl;
	std::cout << "  -Q prog      processing script called at the end of a loop" << std::endl;
	std::cout << "  -?           display this help message" << std::endl;
}

static unsigned int	nImages = 1;
static double		longitude = 0;
static double		latitude = 0;
static unsigned int	period = 1; // one second
static bool	align = false;	// indicates whether exposures should be
				// synchronized with the clock
static bool	timestamped = false;
static double		targetmean = 0;
static double		targetmedian = 0;
static FITSdirectory::filenameformat	format;
static const char	*outpath = ".";
static CallbackPtr	imagecallback;
static CallbackPtr	loopcallback;

/**
 * \brief Loop for night only mode
 *
 * In this mode, we create a new directory every night, and only take images
 * during the night.
 */
void	nightloop(CcdPtr ccd, Exposure& exposure, ExposureTimer& timer) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "night only");

	// we need a sun object for our location to compute sunrise and
	// sunset times
	Sun	sun(longitude, latitude);
	unsigned int	counter = 0;

	// take images until we have enough (which might be without end)
	while ((counter < nImages) || (nImages == 0)) {
		// first compute sunrise and sunset times
		time_t	now;
		time(&now);
		time_t	sunrise = sun.sunrise(now);
		time_t	sunset = sun.sunset(now);

		if (debuglevel == LOG_DEBUG) {
			char	sunrise_s[26];	ctime_r(&sunrise, sunrise_s);
			char	sunset_s[26];	ctime_r(&sunset, sunset_s);
			char	now_s[26];	ctime_r(&now, now_s);
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"sunrise: %24.24s, now: %24.24s, sunset: %24.24s",
				sunrise_s, now_s, sunset_s);
		}

		// there are three situations: 
		// 1. daylight
		// 2. night before midnight
		// 3. night after midnight
		int	nightimages = 0;
		time_t	dirtimestamp;
		if ((sunrise <= now) && (now < sunset)) {
			int	sleeptime = sunset - now;
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"daylight, waiting %d seconds for sunset",
				sleeptime);
			sleep(sleeptime);
		} else {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "night");
			if (now < sunrise) {
				debug(LOG_DEBUG, DEBUG_LOG, 0, "after midnight");
				// compute the directory name from t - 86400
				dirtimestamp = now - 86400;
				// number of images to take till sunrise
				nightimages = (sunrise - now) / period;
			}
			if (sunset < now) {
				debug(LOG_DEBUG, DEBUG_LOG, 0, "before midnight");
				// use now to ocmpute directory name
				dirtimestamp = now;
				// compute sunrise for next day
				sunrise = sun.sunrise(now + 86400);
				nightimages = (sunrise - now) / period;
			}
			FITSdirectory	directory(outpath, dirtimestamp,
				format);
			if (timestamped) {
				if (period >= 120) {
					directory.timestampformat("%H%M");
				} else {
					directory.timestampformat("%H%M%S");
				}
			}
			if (nightimages <= 0) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"no work: %d images", nightimages);
				continue;
			}

			// find out whether we have to take all those images
			// or whether the nImages parameter limits them
			if (nImages) {
				int	maximages = nImages - counter;
				if (maximages < nightimages) {
					nightimages = maximages;
				}
			}
			debug(LOG_DEBUG, DEBUG_LOG, 0, "need to take %d images",
				nightimages);

			// now create the Loop object
			Loop	loop(ccd, exposure, directory);
			loop.period(period);
			loop.nImages(nightimages);
			loop.align(align);
			loop.timer(timer);
			loop.newImageCallback(imagecallback);

			// run the loop
			loop.execute();

			// count the images we have built so far
			counter += loop.counter();

			// execute the end loop programming
			if (loopcallback) {
				// ensure we are waiting for termination of
				// the callback
				ImageProgramCallback	*ipcb
					= dynamic_cast<ImageProgramCallback *>(&*loopcallback);
				if (ipcb) {
					ipcb->wait(true);
				}
				// prepare the argument data
				CallbackDataPtr	cbd(
					new ImageCallbackData(directory.path(),
						ImagePtr()));
				// now call the callback
				(*loopcallback)(cbd);
			}
		}
	}
}

void	loop(CcdPtr ccd, Exposure& exposure, ExposureTimer& timer) {
	// make sure the target directory exists
	FITSdirectory	directory(outpath, format);
	if (timestamped) {
		if (period >= 60) {
			directory.timestampformat("%H%M");
		} else {
			directory.timestampformat("%H%M%S");
		}
	}

	// now create the Loop object
	Loop	loop(ccd, exposure, directory);
	loop.period(period);
	loop.nImages(nImages);
	loop.align(align);
	loop.timer(timer);
	loop.newImageCallback(imagecallback);

	// run the loop
	loop.execute();

	// execute the end loop programming
	if (loopcallback) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "loop callback");
		CallbackDataPtr	cbd(
			new ImageCallbackData(directory.path(), ImagePtr()));
		(*loopcallback)(cbd);
	}
}


int	main(int argc, char *argv[]) {
	debugtimeprecision = 3;
	debugthreads = 1;
	int	c;
	unsigned int	width = 0;
	unsigned int	height = 0;
	unsigned int	xoffset = 0;
	unsigned int	yoffset = 0;
	unsigned int	cameraid = 0;
	unsigned int	ccdid = 0;
	double	exposuretime = 0.1;
	const char	*modulename = "uvc";
	bool	night = false;
	bool	daemonize = true;
	while (EOF != (c = getopt(argc, argv,
			"adw:x:y:w:h:o:C:c:n:e:E:m:p:t?L:l:NFM:P:Q:"))) {
		switch (c) {
		case 'a':
			align = true;
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'h':
			height = atoi(optarg);
			break;
		case 'w':
			width = atoi(optarg);
			break;
		case 'x':
			xoffset = atoi(optarg);
			break;
		case 'y':
			yoffset = atoi(optarg);
			break;
		case 'o':
			outpath = optarg;
			break;
		case 'C':
			cameraid = atoi(optarg);
			break;
		case 'c':
			ccdid = atoi(optarg);
			break;
		case 'n':
			nImages = atoi(optarg);
			break;
		case 'e':
			exposuretime = atof(optarg);
			break;
		case 'm':
			modulename = optarg;
			break;
		case 'E':
			targetmean = atof(optarg);
			break;
		case 'M':
			targetmedian = atof(optarg);
			break;
		case 'p':
			period = atoi(optarg);
			break;
		case 't':
			timestamped = true;
			break;
		case 'l':
			latitude = atof(optarg);
			break;
		case 'L':
			longitude = atof(optarg);
			break;
		case 'N':
			night = true;
			break;
		case 'F':
			daemonize = false;
			break;
		case 'P':
			imagecallback = CallbackPtr(
				new ImageProgramCallback(std::string(optarg)));
			break;
		case 'Q':
			loopcallback = CallbackPtr(
				new ImageProgramCallback(std::string(optarg)));
			break;
		}
	}

	// if E is set, and the initial exposure time is zero, then
	// we should change it to something more reasonable
	if (((0 != targetmean) || (0 != targetmedian)) && (exposuretime == 0)) {
		std::string	msg("cannot change exposure time dynamically "
			"starting from 0");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// daemonize
	if (daemonize) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "daemonizing");
		pid_t	pid = fork();
		if (pid < 0) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot fork: %s",
				strerror(errno));
			return EXIT_FAILURE;
		}
		if (pid > 0) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "parent exit");
			return EXIT_SUCCESS;
		}
		setsid();
		umask(022);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "child process started");
	}

	// what format for the file names is expected?
	format = (timestamped)	? FITSdirectory::BOTH
				: FITSdirectory::COUNTER;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "format: %d", format);

	// load the module
	Repository      repository;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "recovering module '%s'", modulename);
	ModulePtr       module = repository.getModule(modulename);
	module->open();

        // get the camera list
	DeviceLocatorPtr        locator = module->getDeviceLocator();
	std::vector<std::string>        cameras = locator->getDevicelist();
	if (cameraid >= cameras.size()) {
		std::string	msg = stringprintf("camera id %d out of range",
			cameraid);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::range_error(msg);
	}

	// get the camera
	std::string	cameraname = cameras[cameraid];
	CameraPtr	camera = locator->getCamera(cameraname);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got camera %s", cameraname.c_str());

	// get the ccd
        CcdPtr  ccd = camera->getCcd(ccdid);
        debug(LOG_DEBUG, DEBUG_LOG, 0, "got a ccd: %s",
                ccd->getInfo().toString().c_str());

	// find a fitting image rectangle, initialize the exposure structure
        if (width == 0) {
                width = ccd->getInfo().size().width();
        }
        if (height == 0) {
                height = ccd->getInfo().size().height();
        }
        ImageRectangle  imagerectangle = ccd->getInfo().clipRectangle(
                ImageRectangle(ImagePoint(xoffset, yoffset),
                        ImageSize(width, height)));
	Exposure	exposure(imagerectangle, exposuretime);

	// depending on the target values, construct a timer
	ExposureTimer	timer;
	if (targetmean > 0) {
		timer = ExposureTimer(exposure.exposuretime,
			targetmean, ExposureTimer::MEAN);
	} else if (targetmedian > 0) {
		timer = ExposureTimer(exposure.exposuretime,
			targetmedian, ExposureTimer::MEDIAN);
	}

	// if night only was requested, then we need a Sun object, and
	// we have to find out 
	if (night) {
		nightloop(ccd, exposure, timer);
	} else {
		loop(ccd, exposure, timer);
	}

	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main(argc, argv);
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "%s terminated: %s",
			argv[0], x.what());
	}
}
