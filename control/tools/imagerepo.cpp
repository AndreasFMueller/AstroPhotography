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
#include <AstroIO.h>
#include <stacktrace.h>

using namespace astro::config;
using namespace astro::project;
using namespace astro::io;
using namespace astro::camera;

namespace astro {

bool	verbose = false;

/**
 * \brief Command to add an image to the repository
 */
int	command_add(const std::string& reponame,
		const std::vector<std::string>& arguments) {
	// the next argument must be the image file name
	if (arguments.size() < 3) {
		throw std::runtime_error("no image to add specified");
	}
	std::string	imagefilename = arguments[2];
	FITSin	in(imagefilename);
	ImagePtr	image = in.read();
	Configuration::get()->repo(reponame).save(image);
	return EXIT_SUCCESS;
}	

/**
 * \brief command to list the contents of a repository
 */
int	command_list(const std::string& reponame) {
	ImageRepo	repo = Configuration::get()->repo(reponame);
	std::set<ImageEnvelope>	images = repo.get(ImageSpec());
	if (images.size() == 0) {
		return EXIT_SUCCESS;
	}
	std::cout << "id   instrument size       purpose bin   exp  temp observation    project  ";
	if (verbose) {
		std::cout << "UUID                                ";
		std::cout << "filename";
	}
	std::cout << std::endl;

	std::for_each(images.begin(), images.end(),
		[](const ImageEnvelope& image) {
			std::cout << stringprintf("%04ld ", image.id());
			std::cout << stringprintf("%-10.10s ",
				image.camera().c_str());
			std::cout << stringprintf("%-11.11s",
				image.size().toString().c_str());
			std::cout << stringprintf("%-8.8s",
				Exposure::purpose2string(image.purpose()).c_str());
			std::cout << stringprintf("%-3.3s ",
				image.binning().toString().substr(1,3).c_str());
			std::cout << stringprintf("%5.0f",
				image.exposuretime());
			std::cout << stringprintf("%6.1f ",
				image.temperature());
			std::cout << timeformat("%d.%m.%y %H:%M ",
				image.observation());
			std::cout << stringprintf("%-8.8s ",
				image.project().c_str());
			if (verbose) {
				std::cout << stringprintf("%-36.36s ",
					((std::string)image.uuid()).c_str());
				std::cout << image.filename();
			}
			std::cout << std::endl;
		}
	);
	return EXIT_SUCCESS;
}

/**
 * \brief Command to retrieve an image from the repository
 */
int	command_get(const std::string& reponame,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 4) {
		throw std::runtime_error("not enough arguments for 'get'");
	}
	int	id = std::stol(arguments[2]);
	std::string	filename = arguments[3];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "extract image to %s", filename.c_str());
	ImagePtr	image = Configuration::get()->repo(reponame).getImage(id);
	FITSout	out(filename);
	out.setPrecious(false);
	out.write(image);
	return EXIT_SUCCESS;
}	

/**
 * \brief Command to remove an image from the repository
 */
int	command_remove(const std::string& reponame,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 3) {
		throw std::runtime_error("missing id argument");
	}
	int	id = std::stol(arguments[2]);
	ImageRepo	repo = Configuration::get()->repo(reponame);
	repo.remove(id);
	return EXIT_SUCCESS;
}

/**
 * \brief Common part of copy or move operation
 */
int	copy_or_move(const std::string& reponame,
		const std::vector<std::string>& arguments, bool copy) {
	if (arguments.size() < 4) {
		throw std::runtime_error("not enough arguments for 'copy'");
	}
	ImageRepo	srcrepo = Configuration::get()->repo(reponame);
	int	id = std::stol(arguments[2]);
	ImagePtr	image = srcrepo.getImage(id);
	std::string	targetrepo = arguments[3];
	Configuration::get()->repo(targetrepo).save(image);
	if (copy) {
		return EXIT_SUCCESS;
	}
	// move operation also deletes the image from the source repo
	srcrepo.remove(id);
	return EXIT_SUCCESS;
}

/**
 * \brief Command to move an image from one repository to another
 */
int	command_move(const std::string& reponame,
		const std::vector<std::string>& arguments) {
	return copy_or_move(reponame, arguments, false);
}

/**
 * \brief Command to copy an image from one repository to another
 */
int	command_copy(const std::string& reponame,
		const std::vector<std::string>& arguments) {
	return copy_or_move(reponame, arguments, true);
}

/**
 * \brief Command to show all info about an image
 */
int	command_show(const std::string& reponame,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 3) {
		throw std::runtime_error("not enough arguments for 'show'");
	}
	int	id = std::stol(arguments[2]);
	ImageRepo	repo = Configuration::get()->repo(reponame);
	ImageEnvelope	image = repo.getEnvelope(id);
	std::cout << "id:              " << image.id() << std::endl;
	std::cout << "filename:        " << image.filename() << std::endl;
	std::cout << "project:         " << image.project() << std::endl;
	std::cout << "created:         " << timeformat("%Y-%m-%d %H:%M:%S",
		image.created()) << std::endl;
	std::cout << "instrument:      " << image.camera() << std::endl;
	std::cout << "size:            " << image.size().toString() << std::endl;
	std::cout << "binning:         " << image.binning().toString() << std::endl;
	std::cout << "exposure time:   " << image.exposuretime() << std::endl;
	std::cout << "CCD temperature: " << image.temperature() << std::endl;
	std::cout << "observation at:  " << timeformat("%Y-%m-%d %H:%M:%S",
		image.observation()) << std::endl;
	std::cout << "UUID:            " << image.uuid() << std::endl;
	if (verbose) {
		std::cout << "FITS headers:" << std::endl;
		std::for_each(image.metadata.begin(), image.metadata.end(),
			[](const ImageMetadata::value_type& mv) {
				std::cout << "    " << mv.second.toString();
				std::cout << std::endl;
			}
		);
	}
	return EXIT_SUCCESS;
}

/**
 * \brief Usage function in 
 */
void	usage(const char *progname) {
	Path	path(progname);
	std::cerr << "Usage:" << std::endl;
	std::cerr << path.basename() << " [ options ] <repo> add <image.fits>";
	std::cerr << std::endl;
	std::cerr << path.basename() << " [ options ] <repo> list" << std::endl;
	std::cerr << path.basename() << " [ options ] <repo> get <id> <image.fits>";
	std::cerr << std::endl;
	std::cerr << path.basename() << " [ options ] <repo> { show | remove } <id>";
	std::cerr << std::endl;
	std::cerr << "add, list, retrieve and delete images in image repository <repo>";
	std::cerr << std::endl;
	std::cerr << path.basename() << " [ options ] <srcrepo> { copy | move } <id> <targetrepo>";
	std::cerr << std::endl;
	std::cerr << "copy or move an image with id <id> from repo <srcrepo> to <targetrepo>";
	std::cerr << std::endl;
	std::cerr << "Options:" << std::endl;
	std::cerr << "  -c,--config=<cfg>    use configuration file <cfg>";
	std::cerr << std::endl;
	std::cerr << "  -d,--debug           increase debug level" << std::endl;
	std::cerr << "  -v,--verbose         show more details in repo listing";
	std::cerr << std::endl;
	std::cerr << "  -h,--help            display this help message";
	std::cerr << std::endl;
}

static struct option	longopts[] = {
{ "config",	required_argument,	NULL,		'c' }, /* 0 */
{ "debug",	no_argument,		NULL,		'd' }, /* 1 */
{ "help",	no_argument,		NULL,		'h' }, /* 2 */
{ "verbose",	no_argument,		NULL,		'v' }, /* 3 */
{ NULL,		0,			NULL,		0   }
};

/**
 * \brief Main function of the imagerepo program
 */
int	main(int argc, char *argv[]) {
	std::string	configfile;
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "c:dhv", longopts,
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
		case 'v':
			verbose = true;
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
	if (command == "remove") {
		return command_remove(reponame, arguments);
	}
	if (command == "move") {
		return command_move(reponame, arguments);
	}
	if (command == "copy") {
		return command_copy(reponame, arguments);
	}
	if (command == "show") {
		return command_show(reponame, arguments);
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
		std::cerr << "terminated by ";
		std::cerr << astro::demangle(typeid(x).name()) << ": ";
		std::cerr << x.what() << std::endl;
	} catch (...) {
		std::cerr << "terminated by unknown exception" << std::endl;
	}
	return EXIT_FAILURE;
}
