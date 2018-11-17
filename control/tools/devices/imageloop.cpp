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
#include <AstroUtils.h>
#include <Sun.h>
#include <iostream>
#include <typeinfo>

using namespace astro;
using namespace astro::io;
using namespace astro::module;
using namespace astro::camera;
using namespace astro::device;
using namespace astro::image::filter;
using namespace astro::task;
using namespace astro::callback;

namespace astro {
namespace app {
namespace imageloop {

/**
 * \brief Usage of the imageloop program
 */
static void	usage(const char *progname) {
	Path	p(progname);
	std::cout << "usage: " << std::endl;
	std::cout << "    " << std::endl;
	std::cout << p.basename() << " [ options ]" << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -d,--debug                 increase deug level" << std::endl;
	std::cout << "  -m,--module=<module>       load camera module" << std::endl;
	std::cout << "  -C,--camera=<cameraid>     which camera to use, default 0" << std::endl;
	std::cout << "  -c,--ccd=<ccdid>           which ccd to use, default 0" << std::endl;
	std::cout << "  -n,--number=<nimages>      number of images to retrieve, 0 means never stop" << std::endl;
	std::cout << "  -p,--period=<period>       image period" << std::endl;
	std::cout << "  -w,--width=<width>         width of image rectangle" << std::endl;
	std::cout << "  -h,--height=<height>       height of image rectangle" << std::endl;
	std::cout << "  -x,--x-offset=<xoffset>    horizontal offset of image rectangle"
		<< std::endl;
	std::cout << "  -y,--y-offset=<yoffset>    vertical offset of image rectangle"
		<< std::endl;
	std::cout << "  -L,--longitude=<longitude> logitude of the camera location"
		<< std::endl;
	std::cout << "  -l,--latitude=<latitude>   latitude of the camera location"
		<< std::endl;
	std::cout << "  -N,--night                 take images during the night only"
		<< std::endl;
	std::cout << "  -o,--outdir=<outdir>       directory where files should be placed"
		<< std::endl;
	std::cout << "  -t,--timestamp             use timestamps as filenames" << std::endl;
	std::cout << "  -e,--exposure=<time>       (initial) exposure time, modified later if target" << std::endl;
	std::cout << "                             mean set" << std::endl;
	std::cout << "  -E,--mean=<mean>           attempt to vary the exposure time in such a way" << std::endl;
	std::cout << "                             that the mean pixel value stays close to <mean>" << std::endl;
	std::cout << "  -M,--median=<median>       attemtp to vary the exposure time in such a way" << std::endl;
	std::cout << "                             that the median pixel value stays close to the" << std::endl;
	std::cout << "                             <median>" << std::endl;
	std::cout << "  -i,--minimum=<min>         minimum exposure time" << std::endl;
	std::cout << "  -F,--foreground            stay in the foreground" << std::endl;
	std::cout << "  -P,--image-callback=<prog> processing script for individual images," << std::endl;
	std::cout << "                             e.g. convert FITS to JPEG" << std::endl;
	std::cout << "  -Q,--loop-callback=<prog>  processing script called at the end of a loop," << std::endl;
	std::cout << "                             e.g. convert image sequence to MPEG movie" << std::endl;
	std::cout << "  -?,--help                  display this help message" << std::endl;
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
	astro::sun::Sun	sun(longitude, latitude);
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
		time_t	dirtimestamp = 0;
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
					new FileImageCallbackData(directory.path(),
						ImagePtr()));
				// now call the callback
				(*loopcallback)(cbd);
			}
		}
	}
}

void	loop(CcdPtr ccd, Exposure& exposure, ExposureTimer& timer) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start exposure: %.3f",
		exposure.exposuretime());
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
			new FileImageCallbackData(directory.path(), ImagePtr()));
		(*loopcallback)(cbd);
	}
}

static struct option	longopts[] = {
{ "align",		no_argument,		NULL,	'a' }, /*  0 */
{ "ccd",		required_argument,	NULL,	'c' }, /*  1 */
{ "cameraid",		required_argument,	NULL,	'C' }, /*  2 */
{ "debug",		no_argument,		NULL,	'd' }, /*  3 */
{ "mean",		required_argument,	NULL,	'E' }, /*  4 */
{ "exposure",		required_argument,	NULL,	'e' }, /*  5 */
{ "foreground",		no_argument,		NULL,	'F' }, /*  6 */
{ "height",		required_argument,	NULL,	'h' }, /*  7 */
{ "minimum",		required_argument,	NULL,	'i' }, /*  8 */
{ "longitude",		required_argument,	NULL,	'L' }, /*  9 */
{ "latitude",		required_argument, 	NULL,	'l' }, /* 10 */
{ "median",		required_argument,	NULL,	'M' }, /* 11 */
{ "module",		required_argument,	NULL,	'm' }, /* 12 */
{ "night",		no_argument,		NULL,	'N' }, /* 13 */
{ "number",		required_argument,	NULL,	'n' }, /* 14 */
{ "outdir",		required_argument,	NULL,	'o' }, /* 15 */
{ "image-callback",	required_argument,	NULL,	'P' }, /* 16 */
{ "period",		required_argument,	NULL,	'p' }, /* 17 */
{ "loop-callback",	required_argument,	NULL,	'Q' }, /* 18 */
{ "timestamp",		no_argument,		NULL,	't' }, /* 19 */
{ "width",		required_argument,	NULL,	'w' }, /* 20 */
{ "x-offset",		required_argument,	NULL,	'x' }, /* 21 */
{ "y-offset",		required_argument,	NULL,	'y' }, /* 22 */
{ "help",		no_argument,		NULL,	'?' }, /* 23 */
{ NULL,			0,			NULL,	 0  }  /* 24 */
};


/**
 * \brief Main function for the imageloop program
 */
int	main(int argc, char *argv[]) {
	debugtimeprecision = 3;
	debugthreads = 1;
	int	c;
	int	longindex;
	unsigned int	width = 0;
	unsigned int	height = 0;
	int	xoffset = 0;
	int	yoffset = 0;
	unsigned int	cameraid = 0;
	unsigned int	ccdid = 0;
	double	exposuretime = 0.1;
	double	minimum = 0.1;
	const char	*modulename = "uvc";
	bool	night = false;
	bool	daemonize = true;
	while (EOF != (c = getopt_long(argc, argv,
			"adw:x:y:w:h:o:C:c:n:e:E:i:m:p:t?L:l:NFM:P:Q:",
			longopts, &longindex))) {
		switch (c) {
		case 'a':
			align = true;
			break;
		case 'C':
			cameraid = atoi(optarg);
			break;
		case 'c':
			ccdid = atoi(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'E':
			targetmean = atof(optarg);
			break;
		case 'e':
			exposuretime = atof(optarg);
			break;
		case 'F':
			daemonize = false;
			break;
		case 'h':
			height = atoi(optarg);
			break;
		case 'i':
			minimum = atof(optarg);
			break;
		case 'L':
			longitude = atof(optarg);
			break;
		case 'l':
			latitude = atof(optarg);
			break;
		case 'M':
			targetmedian = atof(optarg);
			break;
		case 'm':
			modulename = optarg;
			break;
		case 'N':
			night = true;
			break;
		case 'n':
			nImages = atoi(optarg);
			break;
		case 'o':
			outpath = optarg;
			break;
		case 'P':
			imagecallback = CallbackPtr(
				new ImageProgramCallback(std::string(optarg)));
			break;
		case 'p':
			period = atoi(optarg);
			break;
		case 'Q':
			loopcallback = CallbackPtr(
				new ImageProgramCallback(std::string(optarg)));
			break;
		case 't':
			timestamped = true;
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
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		default:
			throw std::runtime_error("unknown option");
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
	ModuleRepositoryPtr	repository = getModuleRepository();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "recovering module '%s'", modulename);
	ModulePtr       module = repository->getModule(modulename);
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
	exposure.shutter(Shutter::CLOSED);

	// depending on the target values, construct a timer
	ExposureTimer	timer;
	if (targetmean > 0) {
		timer = ExposureTimer(exposure.exposuretime(),
			targetmean, ExposureTimer::MEAN);
	} else if (targetmedian > 0) {
		timer = ExposureTimer(exposure.exposuretime(),
			targetmedian, ExposureTimer::MEDIAN);
	}
	timer.minimum(minimum);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure time: %.3f",
		exposure.exposuretime());

	// if night only was requested, then we need a Sun object, and
	// we have to find out 
	if (night) {
		nightloop(ccd, exposure, timer);
	} else {
		loop(ccd, exposure, timer);
	}

	return EXIT_SUCCESS;
}

} // namespace imageloop
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return main_function<astro::app::imageloop::main>(argc, argv);
}
