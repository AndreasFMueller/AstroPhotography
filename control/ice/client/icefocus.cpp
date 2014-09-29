/*
 * icefocus.cpp -- focusing client 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <stdexcept>
#include <iostream>
#include <stacktrace.h>
#include <focusing.h>
#include <typeinfo>
#include <AstroUtils.h>
#include <AstroConfig.h>
#include <AstroDebug.h>
#include <getopt.h>
#include <CommunicatorSingleton.h>

namespace snowstar {

void	usage(const char *progname) {
	astro::Path	path(progname);
	std::cout << path.basename() << " [ options ]" << std::endl;
	std::cout << "perform focusing using ccd and focuser of an instrument";
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -b,--binning=XxY      select XxY binning mode (default 1x1)"
		<< std::endl;
	std::cout << " -c,--config=<cfg>     use configuration from file <cfg>";
	std::cout << std::endl;
	std::cout << " -d,--debug            increase debug level" << std::endl;
	std::cout << " -e,--exposure=<e>     set exposure time to <e>";
	std::cout << std::endl;
	std::cout << " -f,--filter=<f>       use filter numbered <f>, ignored "
		"if the instrument has";
	std::cout << std::endl;
	std::cout << "                       no filter wheel";
	std::cout << std::endl;
	std::cout << " -h,--help             display this help message and exit";
	std::cout << std::endl;
	std::cout << " -i,--instrument=<INS> use instrument named INS";
	std::cout << std::endl;
	std::cout << " --rectangle=<rec>     expose only a subrectangle as "
		"specified by <rec>.";
	std::cout << std::endl;
	std::cout << "                       <rec> must be of the form";
	std::cout << std::endl;
	std::cout << "                       widthxheight@(xoffset,yoffset)";
	std::cout << std::endl;
	std::cout << " -t,--temperature=<t>  cool ccd to temperature <t>, "
		"ignored if the instrument";
	std::cout << std::endl;
	std::cout << "                       has no cooler";
	std::cout << std::endl;
}

static struct option	longopts[] = {
{ "binning",		required_argument,	NULL,	'b' }, /*  0 */
{ "config",		required_argument,	NULL,	'c' }, /*  1 */
{ "debug",		no_argument,		NULL,	'd' }, /*  2 */
{ "exposure",		required_argument,	NULL,	'e' }, /*  3 */
{ "filter",		required_argument,	NULL,	'f' }, /*  4 */
{ "help",		no_argument,		NULL,	'h' }, /*  5 */
{ "instrument",		required_argument,	NULL,	'i' }, /*  6 */
{ "rectangle",		required_argument,	NULL,	'r' }, /*  7 */
{ "temperature",	required_argument,	NULL,	't' }, /*  8 */
{ NULL,			0,			NULL,    0  }
};

int	main(int argc, char *argv[]) {
	snowstar::CommunicatorSingleton	cs(argc, argv);

	std::string	instrumentname;
	double	exposuretime = 1.0;
	double	temperature = std::numeric_limits<double>::quiet_NaN();
	astro::camera::Binning	binning;
	astro::image::ImageRectangle	frame;
	std::string	filtername;

	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "b:c:de:f:hi:r:t:",
		longopts, &longindex)))
		switch (c) {
		case 'b':
			binning = astro::camera::Binning(optarg);
			break;
		case 'c':
			astro::config::Configuration::set_default(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'e':
			exposuretime = std::stod(optarg);
			break;
		case 'f':
			filtername = optarg;
			break;
		case 'h':
			usage(argv[0]);
			break;
		case 'i':
			instrumentname = optarg;
			break;
		case 'r':
			frame = astro::image::ImageRectangle(optarg);
			break;
		case 't':
			temperature = std::stod(optarg);
			break;
		}


	throw std::runtime_error("not implemented");
}

} // namespace snowstar

int	main(int argc, char *argv[]) {
	signal(SIGSEGV, stderr_stacktrace);
	try {
		return snowstar::main(argc, argv);
	} catch (std::exception& x) {
		std::cerr << "terminated by ";
		std::cerr << astro::demangle(typeid(x).name());
		std::cerr << ": " << x.what();
		std::cerr << std::endl;
	} catch (...) {
		std::cerr << "terminated by unknown exception" << std::endl;
	}
	return EXIT_FAILURE;
}

