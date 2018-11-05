/*
 * snowfiles.cpp -- find and list image files
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <AstroIO.h>
#include <IceConversions.h>
#include <CommunicatorSingleton.h>
#include <CommonClientTasks.h>
#include <includes.h>

namespace snowstar {
namespace app {
namespace snowfiles {

static int	command_list(ImagesPrx images) {
	ImageList	l = images->listImages();
	std::for_each(l.begin(), l.end(),
		[](const std::string& n) {
			std::cout << n << std::endl;
		}
	);
	return EXIT_SUCCESS;
}

static int	command_get(ImagesPrx images, const std::string& filename,
			const std::string& localfilename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get %s to local file %s",
		filename.c_str(), localfilename.c_str());
	try {
		ImagePrx	image = images->getImage(filename);
		astro::image::ImagePtr	imageptr = convert(image);
		astro::io::FITSout	outfile(localfilename);
		outfile.write(imageptr);
		return EXIT_SUCCESS;
	} catch (std::exception& x) {
		std::string	msg = astro::stringprintf("cannot retrieve "
			"image %s: %s", filename.c_str(), x.what());
		throw std::runtime_error(msg);
	}
	return EXIT_FAILURE;
}

static int	command_remove(ImagesPrx images, const std::string& filename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remove %s", filename.c_str());
	try {
		images->remove(filename);
	} catch (snowstar::NotFound x) {
		std::string	msg = astro::stringprintf("cannot delete: %s",
			x.cause.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw;
	} catch (const std::exception x) {
		std::string	msg = astro::stringprintf("cannot delete: %s",
			x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw;
	}
	return EXIT_SUCCESS;
}

static int	command_save(ImagesPrx images, const std::string& filename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "saving %s", filename.c_str());
	try {
		astro::io::FITSin	in(filename);
		astro::image::ImagePtr	image = in.read();
		snowstar::ImageFile	imagefile = convertfile(image);
		std::string	file = images->save(imagefile);
		std::cout << "local: " << filename;
		std::cout << ", remote: " << file << std::endl;
		return EXIT_SUCCESS;
	} catch (const std::exception& x) {
		std::string	msg = astro::stringprintf("cannot save image:"
			" %s", x.what());
		throw std::runtime_error(msg);
	}
}

static int	command_repo(ImagesPrx images, const std::string& filename,
			const std::string& reponame) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "saving %s in repo %s",
		filename.c_str(), reponame.c_str());
	try {
		ImagePrx	image = images->getImage(filename);
		image->toRepository(reponame);
		return EXIT_SUCCESS;
	} catch (const std::exception& x) {
		std::string	msg = astro::stringprintf("cannot move %s to "
			"repo %s: %s", filename.c_str(), reponame.c_str(),
			x.what());
		std::cerr << msg << std::endl;
	}
	return EXIT_FAILURE;
}

static void	usage(const std::string& progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] [ <server> ] help" << std::endl;
	std::cout << p << " [ options ] <server> list" << std::endl;
	std::cout << p << " [ options ] <server> get <filename> <localname>";
	std::cout << std::endl;
	std::cout << p << " [ options ] <server> remove <filename>";
	std::cout << std::endl;
	std::cout << p << " [ options ] <server> save <localname> ...";
	std::cout << std::endl;
	std::cout << p << " [ options ] <server> repo <filename> <reponame>";
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << " -d,--debug     increase debug level" << std::endl;
	std::cout << " -h,--help      display this help message and exit"
		<< std::endl;
}

static struct option	longopts[] = {
{ "debug",	no_argument,			NULL,	'd' },
{ "help",	no_argument,			NULL,	'h' },
{ NULL,		0,				NULL,	 0  },
};

int	main(int argc, char *argv[]) {
	debug_set_ident("snowfiles");
	CommunicatorSingleton	communicator(argc, argv);

	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "dh", longopts,
		&longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		}

	astro::ServerName	servername;

	// next comes the servername
	if (argc <= optind) {
		throw std::runtime_error("command missing");
	}
	std::string	command(argv[optind++]);

	// handle the help command
	if (command == "help") {
		usage(argv[0]);
		return EXIT_SUCCESS;
	}

	servername = astro::ServerName(command);

	// next argument must be the command
	if (argc <= optind) {
		throw std::runtime_error("command missing");
	}
	command = std::string(argv[optind++]);

	if (command == "help") {
		usage(argv[0]);
		return EXIT_SUCCESS;
	}
	
	// contact the server
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	Ice::ObjectPrx	base = ic->stringToProxy(servername.connect("Images"));
	ImagesPrx	images = ImagesPrx::checkedCast(base);

	// handle the list command
	if (command == "list") {
		return command_list(images);
	}

	// commands with one argument
	if (argc <= optind) {
		throw std::runtime_error("not enough arguments");
	}
	std::string	filename(argv[optind++]);

	if (command == "remove") {
		return command_remove(images, filename);
	}
	if (command == "save") {
		command_save(images, filename);
		while (optind < argc) {
			filename = std::string(argv[optind++]);
			command_save(images, filename);
		}
		return EXIT_SUCCESS;
	}

	// commands with two arguments
	if (argc <= optind) {
		throw std::runtime_error("not enough arguments");
	}	
	std::string	localfilename(argv[optind++]);

	if (command == "get") {
		return command_get(images, filename, localfilename);
	}
	if (command == "repo") {
		std::string	reponame = localfilename;
		return command_repo(images, filename, reponame);
	}

	return EXIT_FAILURE;
}

} // namespace snowfiles
} // namespace app
} // namespace snowstar

int     main(int argc, char *argv[]) {
        int	rc = astro::main_function<snowstar::app::snowfiles::main>(argc,
			argv);
	snowstar::CommunicatorSingleton::release();
	return rc;
}
