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

using namespace astro;
using namespace astro::config;
using namespace astro::project;
using namespace astro::io;
using namespace astro::camera;

namespace astro {
namespace app {
namespace imagerepo {

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
	for (size_t imageno = 2; imageno < arguments.size(); imageno++) {
		std::string	imagefilename = arguments[imageno];
		debug(LOG_DEBUG, DEBUG_LOG, 0, "adding image '%s'",
			imagefilename.c_str());
		FITSin	in(imagefilename);
		ImagePtr	image = in.read();
		ConfigurationPtr	configuration = Configuration::get();
		ImageRepoConfigurationPtr	imagerepos
			= ImageRepoConfiguration::get(configuration);
		imagerepos->repo(reponame)->save(image);
	}
	return EXIT_SUCCESS;
}	

/**
 * \brief command to list the contents of a repository
 */
int	command_list(const std::string& reponame) {
	ConfigurationPtr	configuration = Configuration::get();
	ImageRepoConfigurationPtr	imagerepos
		= ImageRepoConfiguration::get(configuration);
	ImageRepoPtr	repo = imagerepos->repo(reponame);
	std::set<ImageEnvelope>	images = repo->get(ImageSpec());
	if (images.size() == 0) {
		return EXIT_SUCCESS;
	}
	std::cout << "id   instrument size       purpose bin   exp  temp "
		"observation    project       ";
	if (verbose) {
		std::cout << " UUID                                 ";
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
			if (image.exposuretime() < 10) {
				std::cout << stringprintf("%5.3f",
					image.exposuretime());
			} else if (image.exposuretime() < 100) {
				std::cout << stringprintf("%5.2f",
					image.exposuretime());
			} else if (image.exposuretime() < 1000) {
				std::cout << stringprintf("%5.1f",
					image.exposuretime());
			} else {
				std::cout << stringprintf("%5.0f",
					image.exposuretime());
			}
			std::cout << stringprintf("%6.1f ",
				image.temperature());
			std::cout << timeformat("%d.%m.%y %H:%M ",
				image.observation());
			std::cout << stringprintf("%-14.14s",
				image.project().c_str());
			if (verbose) {
				std::cout << stringprintf(" %-36.36s ",
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
	int	id = -1;
	try {
		id = std::stol(arguments[2]);
	} catch (const std::exception& x) {
		if ("last" != arguments[2]) {
			std::string	cause = stringprintf("argument '%s' "
				"not a number (%s) and not 'last'",
				arguments[2].c_str(), x.what());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
			return EXIT_FAILURE;
		}
	}
	std::string	filename = arguments[3];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "extract image to %s", filename.c_str());
	ConfigurationPtr	configuration = Configuration::get();
	ImageRepoConfigurationPtr	imagerepos
		= ImageRepoConfiguration::get(configuration);
	ImagePtr	image = imagerepos->repo(reponame)->getImage(id);
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
	ConfigurationPtr	configuration = Configuration::get();
	ImageRepoConfigurationPtr	imagerepos
		= ImageRepoConfiguration::get(configuration);
	for (unsigned int i = 2; i < arguments.size(); i++) {
		int	id = std::stol(arguments[i]);
		ImageRepoPtr	repo = imagerepos->repo(reponame);
		repo->remove(id);
	}
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
	ConfigurationPtr	configuration = Configuration::get();
	ImageRepoConfigurationPtr	imagerepos
		= ImageRepoConfiguration::get(configuration);
	ImageRepoPtr	srcrepo = imagerepos->repo(reponame);
	int	id = std::stol(arguments[2]);
	ImagePtr	image = srcrepo->getImage(id);
	std::string	targetrepo = arguments[3];
	imagerepos->repo(targetrepo)->save(image);
	if (copy) {
		return EXIT_SUCCESS;
	}
	// move operation also deletes the image from the source repo
	srcrepo->remove(id);
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
 * \brief Command to replicate images from one repository to another
 */
int	command_replicate(const std::string& srcreponame, 
		const std::vector<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "replication command");
	if (arguments.size() < 3) {
		throw std::runtime_error("destination repo missing");
	}
	std::string	dstreponame = arguments[2];
	RepoReplicator	replicator;
	ConfigurationPtr	configuration = Configuration::get();
	ImageRepoConfigurationPtr	imagerepos
		= ImageRepoConfiguration::get(configuration);
	ImageRepoPtr	srcrepo = imagerepos->repo(srcreponame);
	ImageRepoPtr	dstrepo = imagerepos->repo(dstreponame);
	int	count = replicator.replicate(srcrepo, dstrepo);
	std::cout << "files replicated: " << count << std::endl;
	return EXIT_SUCCESS;
}

/**
 * \brief Command to synchronize two repositories
 */
int	command_synchronize(const std::string& repo1name, 
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 3) {
		throw std::runtime_error("destination repo missing");
	}
	std::string	repo2name = arguments[2];
	RepoReplicator	replicator;
	ConfigurationPtr	configuration = Configuration::get();
	ImageRepoConfigurationPtr	imagerepos
		= ImageRepoConfiguration::get(configuration);
	ImageRepoPtr	repo1 = imagerepos->repo(repo1name);
	ImageRepoPtr	repo2 = imagerepos->repo(repo2name);
	int	count = replicator.replicate(repo1, repo2);
	std::cout << "files synchronized: " << count << std::endl;
	return EXIT_SUCCESS;
}

/**
 * \brief Command to show all info about an image
 */
int	command_show(const std::string& reponame,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 3) {
		throw std::runtime_error("not enough arguments for 'show'");
	}
	ConfigurationPtr	configuration = Configuration::get();
	ImageRepoConfigurationPtr	imagerepos
		= ImageRepoConfiguration::get(configuration);
	for (unsigned int i = 2; i < arguments.size(); i++) {
		int	id = std::stol(arguments[i]);
		ImageRepoPtr	repo = imagerepos->repo(reponame);
		ImageEnvelope	image = repo->getEnvelope(id);
		std::cout << "id:              "
			<< image.id() << std::endl;
		std::cout << "filename:        "
			<< image.filename() << std::endl;
		std::cout << "project:         "
			<< image.project() << std::endl;
		std::cout << "created:         "
			<< timeformat("%Y-%m-%d %H:%M:%S",
				image.created()) << std::endl;
		std::cout << "instrument:      "
			<< image.camera() << std::endl;
		std::cout << "size:            "
			<< image.size().toString() << std::endl;
		std::cout << "binning:         "
			<< image.binning().toString() << std::endl;
		std::cout << "exposure time:   "
			<< image.exposuretime() << std::endl;
		std::cout << "CCD temperature: "
			<< image.temperature() << std::endl;
		std::cout << "observation at:  "
			<< timeformat("%Y-%m-%d %H:%M:%S",
				image.observation()) << std::endl;
		std::cout << "UUID:            " << image.uuid() << std::endl;
		if (verbose) {
			std::cout << "FITS headers:" << std::endl;
			std::for_each(image.metadata.begin(),
				image.metadata.end(),
				[](const ImageMetadata::value_type& mv) {
					std::cout << "    "
						<< mv.second.toString();
					std::cout << std::endl;
				}
			);
		}
	}
	return EXIT_SUCCESS;
}

/**
 * \brief Usage function in 
 */
static void	usage(const char *progname) {
	Path	path(progname);
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << "    " << path.basename() << " [ options ] <repo> add <image.fits> ...";
	std::cout << std::endl;
	std::cout << "    " << path.basename() << " [ options ] <repo> list" << std::endl;
	std::cout << "    " << path.basename() << " [ options ] <repo> get <id> <image.fits>";
	std::cout << std::endl;
	std::cout << "    " << path.basename() << " [ options ] <repo> { show | remove } <ids>";
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "add, list, retrieve and delete images in image repository <repo>. The get ";
	std::cout << std::endl;
	std::cout << "command understands 'last' as the last, i.e. usually the most recent id of";
	std::cout << std::endl;
	std::cout << "the repository";
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "    " << path.basename() << " [ options ] <srcrepo> { copy | move } <id> <targetrepo>";
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "copy or move an image with id <id> from repo <srcrepo> to <targetrepo>";
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "    " << path.basename() << " [ options ] <srcrepo> { replicate | synchronize } <targetrepo>";
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "replicate images from <srcrepo> to <targetrepo>, synchronize two repositories";
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -c,--config=<cfg>    use configuration file <cfg>";
	std::cout << std::endl;
	std::cout << "  -d,--debug           increase debug level" << std::endl;
	std::cout << "  -v,--verbose         show more details in repo listing";
	std::cout << std::endl;
	std::cout << "  -h,--help            display this help message";
	std::cout << std::endl;
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
			return EXIT_SUCCESS;
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
	if (command == "replicate") {
		return command_replicate(reponame, arguments);
	}
	if (command == "synchronize") {
		return command_synchronize(reponame, arguments);
	}

	// get the image server from the configuration
	return EXIT_SUCCESS;
}

} // namespace imagerepo
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::imagerepo::main>(argc, argv);
}
