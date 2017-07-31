/*
 * focusing.cpp -- perform focusing
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <stdexcept>
#include <iostream>
#include <AstroDebug.h>
#include <AstroDevice.h>
#include <AstroLoader.h>
#include <includes.h>
#include <AstroFocus.h>
#include <AstroFilterfunc.h>
#include <AstroFormat.h>
#include <AstroIO.h>

using namespace astro;
using namespace astro::focusing;

namespace astro {
namespace app {
namespace focusing {

class FocusingCallback : public astro::callback::Callback {
	std::string	_prefix;
	int	counter;
public:
	FocusingCallback(const std::string& prefix) : _prefix(prefix) {
		counter = 0;
	}
	astro::callback::CallbackDataPtr	operator()(
		astro::callback::CallbackDataPtr data) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "executing callback");
		if (!data) {
			return data;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "find argument type");
		astro::callback::ImageCallbackData	*i
			= dynamic_cast<astro::callback::ImageCallbackData *>(&*data);
		if (NULL == i) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "not an ImageCallbackData");
			return data;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got an ImageCallbackData");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image size: %s", i->image()->size().toString().c_str());
		std::string	filename
			= _prefix + stringprintf("-%d.fits", counter++);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "file name: %s",
			filename.c_str());
		astro::io::FITSout	outfile(filename);
		outfile.write(i->image());
		return data;
	}
};

static void	usage(const char *progname) {
	Path	p(progname);
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << "    " << p.basename() << " [ options ]" << std::endl;
	std::cout << std::endl;
	std::cout << "Focus using the given CCD and focuser" << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << std::endl;
	std::cout << "    -a,--algorithm=<method>  select focusing method (FWHM,FOM)" << std::endl;
	std::cout << "    -d,--debug               increase debug level" << std::endl;
	std::cout << "    -m,--min=<min>           minimum focuser position" << std::endl;
	std::cout << "    -M,--max=<max>           maximum focuser position" << std::endl;
	std::cout << "    -C,--ccd=<ccdname>       CCD to use for focusing" << std::endl;
	std::cout << "    -s,--steps=<steps>       number of steps to take during focusing" << std::endl;
	std::cout << "    -e,--exposure=<time>     exposure time" << std::endl;
	std::cout << "    -F,--focuser=<name>      focuser name to use" << std::endl;
	std::cout << "    -x,--x=<x>               x coordinate of focusing area" << std::endl;
	std::cout << "    -y,--y=<y>               y coordinate of focusing area" << std::endl;
	std::cout << "    -h,--height=<height>     height of focusing area" << std::endl;
	std::cout << "    -w,--width=<width>       width of focusing area" << std::endl;
	std::cout << std::endl;
}

static struct option	longopts[] = {
{ "algorithm",	required_argument,	NULL,	'a' }, /*  0 */
{ "debug",	required_argument,	NULL,	'd' }, /*  1 */
{ "min",	required_argument,	NULL,	'm' }, /*  2 */
{ "max",	required_argument,	NULL,	'M' }, /*  3 */
{ "ccd",	required_argument,	NULL,	'C' }, /*  4 */
{ "steps",	required_argument,	NULL,	's' }, /*  5 */
{ "exposure",	required_argument,	NULL,	'e' }, /*  6 */
{ "focuser",	required_argument,	NULL,	'F' }, /*  7 */
{ "x",		required_argument,	NULL,	'x' }, /*  8 */
{ "y",		required_argument,	NULL,	'y' }, /*  9 */
{ "height",	required_argument,	NULL,	'h' }, /* 10 */
{ "width",	required_argument,	NULL,	'w' }, /* 11 */
{ "help",	required_argument,	NULL,	'?' }, /* 12 */
{ NULL,		0,			NULL,	 0  },
};

int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	long	min = 24000;
	long	max = 40000;
	int	steps = 10;
	DeviceName	ccdname("ccd:simulator/camera/ccd");
	DeviceName	focusername("focuser:simulator/focuser");
	double	exposuretime = 1;
	int	x = -1;
	int	y = -1;
	int	width = -1;
	int	height = -1;
	Focusing::method_type	method = Focusing::FWHM;
	while (EOF != (c = getopt_long(argc, argv, "dm:M:C:F:s:e:x:y:w:h:a:",
		longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			debugthreads = 1;
			break;
		case 'm':
			min = atoi(optarg);
			break;
		case 'M':
			max = atoi(optarg);
			break;
		case 'C':
			ccdname = DeviceName(optarg);
			break;
		case 'F':
			focusername = DeviceName(optarg);
			break;
		case 's':
			steps = atoi(optarg);
			break;
		case 'e':
			exposuretime = atof(optarg);
			break;
		case 'x':
			x = atoi(optarg);
			break;
		case 'y':
			y = atoi(optarg);
			break;
		case 'w':
			width = atoi(optarg);
			break;
		case 'h':
			height = atoi(optarg);
			break;
		case 'a':
			method = Focusing::string2method(optarg);
			break;
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		default:
			throw std::runtime_error("unknown option");
		}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "start focusing process");

	// get the device repository
	astro::module::Repository	repository;
	astro::module::Devices	devices(repository);

	// get the ccd
	astro::camera::CcdPtr	ccd = devices.getCcd(ccdname);

	// get missing information from the CCD
	if (x < 0) { x = ccd->getInfo().size().width() / 2; }
	if (y < 0) { y = ccd->getInfo().size().height() / 2; }
	if (width < 0) { width = ccd->getInfo().size().width(); }
	if (height < 0) { height = ccd->getInfo().size().height(); }

	// get the focuser
	astro::camera::FocuserPtr	focuser
		= devices.getFocuser(focusername);
	if (focuser->min() > min) {
		std::cerr << "valid range starts at " << focuser->min()
			<< ", " << min << " too small" << std::endl;
		return EXIT_FAILURE;
	}
	if (focuser->max() < max) {
		std::cerr << "valid range ends at " << focuser->max()
			<< ", " << max << " too large" << std::endl;
		return EXIT_FAILURE;
	}
	sleep(2); // to ensure the filter wheel is idle for the simulator

	// prepare the exposure 
	astro::camera::Exposure	exposure;
	exposure.exposuretime(exposuretime);
	exposure.frame(ImageRectangle(
		ImagePoint(x - width / 2, y - height / 2),
		ImageSize(width, height)));
	exposure.shutter(astro::camera::Shutter::OPEN);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure: %s",
		exposure.toString().c_str());

	// run the focusing process
	Focusing	focusing(ccd, focuser);
	focusing.exposure(exposure);
	focusing.steps(steps);
	focusing.method(method);

	// install the callback
	astro::callback::CallbackPtr	cbptr = astro::callback::CallbackPtr(
		new FocusingCallback(std::string("fc/image")));
	focusing.callback(cbptr);

	// now start the process
	focusing.start(min, max);

	// wait until focusing is complete
	while (!focusing.completed()) {
		std::string	statusname
			= Focusing::state2string(focusing.status());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "focusing status: %s",
			statusname.c_str());
		sleep(1);
	}

	Focusing::state_type	state = focusing.status();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "focusing process complete: %s",
		Focusing::state2string(state).c_str());
	return (Focusing::FOCUSED == state) ? EXIT_SUCCESS : EXIT_FAILURE;
}

} // namespace focusing
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::focusing::main>(argc, argv);
}
