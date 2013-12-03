/*
 * guide.cpp -- guide command line client
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#if _HAVE_CONFIG_H
#include <config.h>
#endif /* _HAVE_CONFIG_H */

#include <cstdlib>
#include <cstdio>
#include <AstroDebug.h>
#include <stdexcept>
#include <unistd.h>
#include <iostream>
#include <OrbSingleton.h>
#include <NameService.h>
#include <cerrno>
#include <cstring>
#include <guidecli.h>
#include <module.hh>
#include <CorbaExceptionReporter.h>

extern int	yydebug;

namespace astro {

int	main(int argc, char *argv[]) {
	Astro::OrbSingleton	orb(argc, argv);

	int	c;
	char	*filename = NULL;
	while (EOF != (c = getopt(argc, argv, "df:")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'f':
			filename = optarg;
			break;
		}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "guide program started");

	// create the command line interpreter
	astro::cli::commandfactory	factory;
	astro::cli::guidecli	cli(factory);
	astro::cli::guidesharedcli	s(&cli);

	// get a reference to the naming service
	Astro::Naming::NameService	nameservice(orb);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got naming service");

	// Next we want to get a reference to the Modules object
	cli.modules = orb.getModules();
	if (CORBA::is_nil(cli.modules)) {
		throw std::runtime_error("nil object reference");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a reference to a Modules object");

	// Next we want to get a reference to the Images object
	cli.images = orb.getImages();
	if (CORBA::is_nil(cli.images)) {
		throw std::runtime_error("nil object reference");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a reference to a Images object");

	// next we want to get a reference to the TaskQueue object
	cli.taskqueue = orb.getTaskQueue();
	if (CORBA::is_nil(cli.taskqueue)) {
		throw std::runtime_error("nil object reference");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a reference to a TaskQueue object");

	/* start parsing the input */
	if (filename) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "parsing '%s'", filename);
		cli.parse(filename);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "parsing stdin");
		cli.prompt("> ");
		std::cout << cli.prompt();
		cli.parse(&std::cin);
	}

	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main(argc, argv);
	} catch (CORBA::Exception& x) {
		std::string	s = Astro::exception2string(x);
		std::cerr << "guide program terminated by Corba exception: "
			<< s << std::endl;
	} catch (std::exception& x) {
		std::cerr << "guide program terminated by exception: "
			<< x.what() << std::endl;
	} catch (...) {
		std::cerr << "guide program terminated by unknown exception"
			<< std::endl;
	}
	exit(EXIT_FAILURE);
}
