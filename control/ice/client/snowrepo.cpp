/*
 * snowrepo.cpp -- image repository replication tool
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CommunicatorSingleton.h>
#include <getopt.h>
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <AstroConfig.h>
#include <AstroFormat.h>
#include <repository.h>

using namespace snowstar;

namespace snowrepo {

std::string	server("localhost");

void	usage(const char *progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] help" << std::endl;
	std::cout << p << " [ options ] list" << std::endl;
	std::cout << p << " [ options ] { push | pull | synchronize } <localrepo> <remoterepo>" << std::endl;
	std::cout << std::endl;
	std::cout << "replicate files from a local repository to a remote repository" << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -c,--config=<cfg>       use configuration <cfg>" << std::endl;
	std::cout << " -d,--debug              increase debug level" << std::endl;
	std::cout << " -s,--server=<server>    connect to repositories on server <server>" << std::endl;
	std::cout << " -h,--help               display this help and exit" << std::endl;
	std::cout << std::endl;
}

int	command_help() {
	std::cout << "This command helps keeping local and remote image repositories in sync." << std::endl;
	return EXIT_SUCCESS;
}

int	command_list(RepositoriesPrx repositories) {
	reponamelist names = repositories->list();
	std::for_each(names.begin(), names.end(),
		[](const std::string& name) {
			std::cout << name << std::endl;
		}
	);
	return EXIT_SUCCESS;
}

static struct option	longopts[] = {
{ "config",	required_argument,	NULL,	'c' }, /* 0 */
{ "debug",	no_argument,		NULL,	'd' }, /* 1 */
{ "help",	no_argument,		NULL,	'h' }, /* 2 */
{ "server",	required_argument,	NULL,	's' }, /* r */
{ NULL,		0,			NULL,	0   }
};

int	main(int argc, char *argv[]) {
	CommunicatorSingleton	communicator(argc, argv);

	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "c:dhs:",
		longopts, &longindex)))
		switch (c) {
		case 'c':
			astro::config::Configuration::set_default(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 's':
			server = optarg;
			break;
		}

	// next argument must be the command
	if (argc <= optind) {
		throw std::runtime_error("command argument missing");
	}
	std::string	command = argv[optind++];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "command: %s", command.c_str());

	// help command can be execute now
	if (command == "help") {
		return command_help();
	}

	// all other commands need a remote Repositories reference
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	astro::ServerName	servername(server);
	std::string	connectstring
		= astro::stringprintf("Repositories:default -h %s -p %hu",
			servername.host().c_str(), servername.port());
	Ice::ObjectPrx	base = ic->stringToProxy(connectstring);
	RepositoriesPrx	repositories = RepositoriesPrx::checkedCast(base);
	if (!repositories) {
		throw std::runtime_error("no repositories proxy");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "connected to %s",
		connectstring.c_str());

	// list command needs nothing more
	if (command == "list") {
		return command_list(repositories);
	}

	// remaining commands need a local and a remote repository name
	if (argc <= optind) {
		throw std::runtime_error("local repository name missing");
	}
	std::string	localreponame = argv[optind++];
	if (argc <= optind) {
		throw std::runtime_error("remote repository name missing");
	}
	std::string	remotereponame = argv[optind++];

	// get the local repository

	// get a remote repository reference

	// 

	throw std::runtime_error("incomplete implementation");
}

} // namespace snowrepo

int main(int argc, char *argv[]) {
	return astro::main_function<snowrepo::main>(argc, argv);
}
