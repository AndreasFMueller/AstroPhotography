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
#include <IceConversions.h>

using namespace snowstar;

namespace snowstar {
namespace app {
namespace snowrepo {

std::string	server("localhost");
bool	dryrun = false;
bool	verbose = false;

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
	std::cout << " -n,--dry-run            don't do anything, just report on what would be done" << std::endl;
	std::cout << " -s,--server=<server>    connect to repositories on server <server>" << std::endl;
	std::cout << " -h,--help               display this help and exit" << std::endl;
	std::cout << " -v,--verbose            give more information about what is being done" << std::endl;
	std::cout << std::endl;
}

/**
 * \brief Help command implementation
 */
int	command_help() {
	std::cout << "This command helps keeping local and remote image repositories in sync." << std::endl;
	return EXIT_SUCCESS;
}

/**
 * \brief List command implementation
 */
int	command_list(RepositoriesPrx repositories) {
	reponamelist names = repositories->list();
	std::for_each(names.begin(), names.end(),
		[](const std::string& name) {
			std::cout << name << std::endl;
		}
	);
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the push command
 */
int	command_push(astro::project::ImageRepo& localrepo,
		const std::set<std::string>& localuuids,
		RepositoryPrx remoterepo,
		const std::set<std::string>& remoteuuids) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "push command");
	// get the uuids that should be copied
	std::list<std::string>	tocopy;
	std::set_difference(localuuids.begin(), localuuids.end(),
		remoteuuids.begin(), remoteuuids.end(),
		std::inserter(tocopy, tocopy.begin()));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d uuids to copy", tocopy.size());
	
	// get the ids for the images
	std::set<int>	copyids;
	for (auto ptr = tocopy.begin(); ptr != tocopy.end(); ptr++) {
		copyids.insert(localrepo.getId(astro::UUID(*ptr)));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d ids to copy", copyids.size());
	if (verbose) {
		std::cout << "found " << copyids.size();
		std::cout << " files identified for push operation";
		std::cout << std::endl;
	}

	// now copy all the images to the remote repo
	for (auto ptr = copyids.begin(); ptr != copyids.end(); ptr++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "copy image id %d", *ptr);
		if (verbose) {
			std::cout << "copy file " << *ptr << std::endl;
		}
		if (!dryrun) {
			remoterepo->save(convertfile(localrepo.getImage(*ptr)));
		}
	}

	// replication complete
	debug(LOG_DEBUG, DEBUG_LOG, 0, "push complete");
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the pull command
 */
int	command_pull(astro::project::ImageRepo& localrepo,
		const std::set<std::string>& localuuids,
		RepositoryPrx remoterepo,
		const std::set<std::string>& remoteuuids) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "pull command");
	// get the uuids that should be copied
	std::list<std::string>	tocopy;
	std::set_difference(remoteuuids.begin(), remoteuuids.end(),
		localuuids.begin(), localuuids.end(),
		std::inserter(tocopy, tocopy.begin()));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d uuids to copy", tocopy.size());
	
	// get the ids for the images
	std::set<int>	copyids;
	for (auto ptr = tocopy.begin(); ptr != tocopy.end(); ptr++) {
		copyids.insert(remoterepo->getId(*ptr));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d ids to copy", copyids.size());
	if (verbose) {
		std::cout << "found " << copyids.size();
		std::cout << " files identified for pull operation";
		std::cout << std::endl;
	}

	// copy the images from the remote repository
	for (auto ptr = copyids.begin(); ptr != copyids.end(); ptr++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "copy image id %d", *ptr);
		if (verbose) {
			std::cout << "pulling " << *ptr << std::endl;
		}
		if (!dryrun) {
			ImageFile	imagefile = remoterepo->getImage(*ptr);
			localrepo.save(convertfile(imagefile));
		}
	}

	// replication complete
	debug(LOG_DEBUG, DEBUG_LOG, 0, "pull complete");
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the synchronization command
 */
int	command_sync(astro::project::ImageRepo& localrepo,
		const std::set<std::string>& localuuids,
		RepositoryPrx remoterepo,
		const std::set<std::string>& remoteuuids) {
	int	rc;
	rc = command_push(localrepo, localuuids, remoterepo, remoteuuids);
	if (rc != EXIT_SUCCESS) {
		throw std::runtime_error("cannot push images");
	}
	rc = command_pull(localrepo, localuuids, remoterepo, remoteuuids);
	if (rc != EXIT_SUCCESS) {
		throw std::runtime_error("cannot push images");
	}
	return EXIT_SUCCESS;
}

static struct option	longopts[] = {
{ "config",	required_argument,	NULL,	'c' }, /* 0 */
{ "debug",	no_argument,		NULL,	'd' }, /* 1 */
{ "help",	no_argument,		NULL,	'h' }, /* 2 */
{ "dry-run",	no_argument,		NULL,	'n' }, /* 3 */
{ "server",	required_argument,	NULL,	's' }, /* 4 */
{ "verbose",	no_argument,		NULL,	'v' }, /* 5 */
{ NULL,		0,			NULL,	0   }
};

/**
 * \brief Main function for the snowrepo program
 */
int	main(int argc, char *argv[]) {
	CommunicatorSingleton	communicator(argc, argv);

	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "c:dhs:v",
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
		case 'n':
			dryrun = true;
			break;
		case 's':
			server = optarg;
			break;
		case 'v':
			verbose = true;
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
	Ice::ObjectPrx	base = ic->stringToProxy(
					servername.connect("Repositories"));
	RepositoriesPrx	repositories = RepositoriesPrx::checkedCast(base);
	if (!repositories) {
		throw std::runtime_error("no repositories proxy");
	}

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
	astro::config::ConfigurationPtr	config
		= astro::config::Configuration::get();
	astro::project::ImageRepo	localrepo = config->repo(localreponame);
	std::set<astro::UUID>	uuids = localrepo.getUUIDs("0 = 0");
	std::set<std::string>	localuuids;
	std::copy(uuids.begin(), uuids.end(),
		std::inserter(localuuids, localuuids.begin()));
	if (verbose) {
		std::cout << "found " << localuuids.size() << " files in local repo" << std::endl;
	}

	// get a remote repository reference
	RepositoryPrx	remoterepo = repositories->get(remotereponame);
	uuidlist u = remoterepo->getUUIDs();
	std::set<std::string>	remoteuuids;
	std::copy(u.begin(), u.end(),
		std::inserter(remoteuuids, remoteuuids.begin()));
	if (verbose) {
		std::cout << "found " << remoteuuids.size() << " files in remote repo" << std::endl;
	}

	// push/pull/synchronize commands
	if (command == "push") {
		return command_push(localrepo, localuuids,
				remoterepo, remoteuuids);
	}
	if (command == "pull") {
		return command_pull(localrepo, localuuids,
				remoterepo, remoteuuids);
	}
	if (command == "sync") {
		return command_sync(localrepo, localuuids,
				remoterepo, remoteuuids);
	}

	throw std::runtime_error("unknown command");
}

} // namespace snowrepo
} // namespace app
} // namespace snowstar

int main(int argc, char *argv[]) {
	return astro::main_function<snowstar::app::snowrepo::main>(argc, argv);
}
