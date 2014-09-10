/*
 * imagerepo.cpp -- image repository client tool
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <typeinfo>
#include <cstdlib>
#include <cstdio>
#include <stdexcept>
#include <iostream>
#include <includes.h>
#include <AstroConfig.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroUtils.h>
#include <stacktrace.h>

using namespace astro::config;
using namespace astro::project;

namespace astro {

int	command_add(const std::string& /* reponame */,
		const std::vector<std::string>& arguments) {
	std::cerr << "'add' command not implemented" << std::endl;
	return EXIT_FAILURE;
}	

int	command_list(const std::string& reponame) {
	ImageRepo	repo = Configuration::get()->repo(reponame);
	std::set<ImageEnvelope>	images = repo.get(ImageSpec());
	if (images.size() == 0) {
		return EXIT_SUCCESS;
	}
	std::cout << "[ id ] camera   size       bin   exp  temp observation    project " << std::endl;

	std::for_each(images.begin(), images.end(),
		[](const ImageEnvelope& image) {
			std::cout << stringprintf("[%04ld] ", image.id());
			std::cout << stringprintf("%-8.8s ",
				image.camera().c_str());
			std::cout << stringprintf("%-11.11s",
				image.size().toString().c_str());
			std::cout << stringprintf("%-3.3s ",
				image.binning().toString().substr(1,3).c_str());
			std::cout << stringprintf("%5.0f",
				image.exposuretime());
			std::cout << stringprintf("%6.1f ",
				image.temperature());
			std::cout << timeformat("%d.%m.%y %H:%m ",
				image.observation());
			std::cout << stringprintf("%-8.8s ",
				image.project().c_str());
			std::cout << image.filename();
			std::cout << std::endl;
		}
	);
	return EXIT_SUCCESS;
}	

int	command_get(const std::string& reponame,
		const std::vector<std::string>& arguments) {
	std::cerr << "'get' command not implemented" << std::endl;
	return EXIT_FAILURE;
}	

/**
 * \brief Usage function in 
 */
void	usage(const char *progname) {
	std::cerr << "usage:" << std::endl;
	std::cerr << progname << " [ options ] <server> add <image.fits>";
	std::cerr << std::endl;
	std::cerr << progname << " [ options ] <server> list" << std::endl;
	std::cerr << progname << " [ options ] <server> get <id> <image.fits>";
	std::cerr << std::endl;
	std::cerr << "add, list and retrieve images in image server <server>";
	std::cerr << std::endl;
	std::cerr << "optoins:" << std::endl;
	std::cerr << "  -c,--config=<cfg>    use configuration file <cfg>";
	std::cerr << std::endl;
	std::cerr << "  -d,--debug           increase debug level" << std::endl;
	std::cerr << "  -h,--help            display this help message";
	std::cerr << std::endl;
}

static struct option	longopts[] = {
{ "config",	required_argument,	NULL,		'c' }, /* 0 */
{ "debug",	no_argument,		NULL,		'd' }, /* 1 */
{ "help",	no_argument,		NULL,		'h' }, /* 2 */
{ NULL,		0,			NULL,		0   }
};

/**
 * \brief Main function of the imagerepo program
 */
int	main(int argc, char *argv[]) {
	std::string	configfile;
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "c:dh", longopts,
		&longindex))) {
		switch (c) {
		case 'c':
			configfile = std::string(optarg);
			Configuration::set_default(configfile);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
			usage(argv[0]);
			break;
		case 1:
			switch (longindex) {
			}
			break;
		}
	}

	// build the arguments vector
	std::vector<std::string>	arguments;
	while (optind < argc) {
		arguments.push_back(std::string(argv[optind++]));
	}

	// next argument must be the image server name
	if (arguments.size() < 2) {
		std::cerr << "no image server name argument" << std::endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	}
	std::string	reponame = arguments[0];
	std::string	command = arguments[1];
	if (command == "add") {
		return command_add(reponame, arguments);
	}
	if (command == "list") {
		return command_list(reponame);
	}
	if (command == "get") {
		return command_get(reponame, arguments);
	}

	// get the image server from the configuration
	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	signal(SIGSEGV, stderr_stacktrace);
	try {
		return astro::main(argc, argv);
	} catch (const std::exception& x) {
		std::cerr << "terminated by " << typeid(x).name() << ": ";
		std::cerr << x.what() << std::endl;
	} catch (...) {
		std::cerr << "terminated by unknown exception" << std::endl;
	}
	return EXIT_FAILURE;
}
