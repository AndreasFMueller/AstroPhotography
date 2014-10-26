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
	std::cout << " -s,--server=<server>    connect to repositories on server <server>, which" << std::endl;
	std::cout << "                         can either be a simple server name or a string" << std::endl;
	std::cout << "                         of the form <host>:<port>" << std::endl;
	std::cout << " -h,--help               display this help and exit" << std::endl;
	std::cout << " -v,--verbose            give more information about what is being done" << std::endl;
	std::cout << std::endl;
}

/**
 * \brief Help command implementation
 */
int	command_help(const char *progname) {
	std::cout << "snowrepo: keep image repos in sync" << std::endl;
	std::cout << std::endl;
	std::cout << "The snowrepo command helps keeping image repositories "
		"in sync." << std::endl;
	std::cout << "All images in a repository are identifiable using their "
		"UUID. When comparing" << std::endl;
	std::cout << "two repositories, the snorepo program uses the UUID to "
		"decide which of the" << std::endl;
	std::cout << "images are only contained in one of the repositories and "
		"should thus" << std::endl;
	std::cout << "replicated." << std::endl;
	std::cout << std::endl;
	std::cout << "The snowrepo program can synchronize a local or remote "
		"repository with" << std::endl;
	std::cout << "a remote repository. A remote repository can be "
		"specified using the --server" << std::endl;
	std::cout << "option, in this case the <remoterepo> argument to the "
		"push, pull and sync" << std::endl;
	std::cout << "commands is simply the repository name in this case. "
		"When using two remote" << std::endl;
	std::cout << "repositories, both have to be specified using a "
		"repository URL of the form" << std::endl;
	std::cout << "repo://<hostname>:<port>/<reponame>" << std::endl;
	std::cout << std::endl;
	usage(progname);
	return EXIT_SUCCESS;
}

/**
 * \brief List command implementation
 */
int	command_list(const astro::ServerName& servername) {
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	Ice::ObjectPrx	base = ic->stringToProxy(
					servername.connect("Repositories"));
	RepositoriesPrx	repositories = RepositoriesPrx::checkedCast(base);
	if (!repositories) {
		throw std::runtime_error("cannot connect to remote server");
	}
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
{ "dry-run",	no_argument,		NULL,	'n' }, /* 3 */
{ "server",	required_argument,	NULL,	's' }, /* 4 */
{ "verbose",	no_argument,		NULL,	'v' }, /* 5 */
{ NULL,		0,			NULL,	0   }
};

/**
 * \brief base replicator class
 */
class BaseRepoReplicator {
protected:
	RepositoriesPrx	remoterepositories;
	RepositoryPrx	remoterepository;
	std::set<std::string>	remoteuuids;
	std::set<std::string>	getUUIDs(RepositoryPrx repo) {
		uuidlist u = repo->getUUIDs();
		std::set<std::string>	remoteuuids;
		std::copy(u.begin(), u.end(),
			std::inserter(remoteuuids, remoteuuids.begin()));
		return remoteuuids;
	}
	std::set<std::string>	localuuids;
public:
	BaseRepoReplicator(const astro::URL& url) {
		Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
		Ice::ObjectPrx	base = ic->stringToProxy(
						url.connect("Repositories"));
		remoterepositories = RepositoriesPrx::checkedCast(base);
		if (!remoterepositories) {
			throw std::runtime_error("no repositories proxy");
		}
		remoterepository = remoterepositories->get(url.path());
		if (!remoterepository) {
			throw std::runtime_error("no repository proxy");
		}
		remoteuuids = getUUIDs(remoterepository);
	}
	virtual std::set<int>	getlocalids(const std::list<std::string>& tocopy) const = 0;
	virtual void	push(const int id) = 0;
	virtual void	pull(const int id) = 0;
	int	push() {
		// get the uuids that should be copied
		std::list<std::string>	tocopy;
		std::set_difference(localuuids.begin(), localuuids.end(),
			remoteuuids.begin(), remoteuuids.end(),
			std::inserter(tocopy, tocopy.begin()));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d uuids to copy",
			tocopy.size());
		
		// get the ids for the images
		std::set<int>	copyids = getlocalids(tocopy);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d ids to copy",
			copyids.size());
		if (verbose) {
			std::cout << "found " << copyids.size();
			std::cout << " files identified for push operation";
			std::cout << std::endl;
		}

		// now copy all the images to the remote repo
		for (auto ptr = copyids.begin(); ptr != copyids.end(); ptr++) {
			push(*ptr);
		}
		return EXIT_SUCCESS;
	}
	int	pull() {
		// get the uuids that should be copied
		std::list<std::string>	tocopy;
		std::set_difference(remoteuuids.begin(), remoteuuids.end(),
			localuuids.begin(), localuuids.end(),
			std::inserter(tocopy, tocopy.begin()));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d uuids to copy",
			tocopy.size());
		
		// get the ids for the images
		std::set<int>	copyids;
		for (auto ptr = tocopy.begin(); ptr != tocopy.end(); ptr++) {
			copyids.insert(remoterepository->getId(*ptr));
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d ids to copy",
			copyids.size());
		if (verbose) {
			std::cout << "found " << copyids.size();
			std::cout << " files identified for pull operation";
			std::cout << std::endl;
		}

		// copy the images from the remote repository
		for (auto ptr = copyids.begin(); ptr != copyids.end(); ptr++) {
			pull(*ptr);
		}
		return EXIT_SUCCESS;
	}
	int	sync() {
		int	rc = push();
		if (rc) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "failed to push");
			return EXIT_FAILURE;
		}
		rc = pull();
		if (rc) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "failed to pull");
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	}
	int	command(const std::string& commandname) {
		if (commandname == "push") {
			return push();
		}
		if (commandname == "pull") {
			return pull();
		}
		if (commandname == "sync") {
			return sync();
		}
		std::cerr << "unknown command";
		return EXIT_FAILURE;
	}
};

/**
 * \brief Replicator class to replicate a local repository with a remote repo
 */
class LocalRepoReplicator : public BaseRepoReplicator {
protected:
	astro::project::ImageRepoPtr	localrepository;
	std::set<std::string>	getUUIDs(astro::project::ImageRepo& repo) {
		std::set<astro::UUID>	uuids = repo.getUUIDs("0 = 0");
		std::set<std::string>	localuuids;
		std::copy(uuids.begin(), uuids.end(),
			std::inserter(localuuids, localuuids.begin()));
		return localuuids;
	}
public:
	LocalRepoReplicator(const std::string& localreponame,
		const astro::URL& remoteurl) : BaseRepoReplicator(remoteurl) {
		astro::config::ConfigurationPtr	config
			= astro::config::Configuration::get();
		localrepository = config->repo(localreponame);
		localuuids = getUUIDs(*localrepository);
	}
	virtual std::set<int>	getlocalids(const std::list<std::string>& tocopy) const {
		std::set<int>	copyids;
		for (auto ptr = tocopy.begin(); ptr != tocopy.end(); ptr++) {
			copyids.insert(localrepository->getId(astro::UUID(*ptr)));
		}
		return copyids;
	}
	virtual void	push(int id) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "copy image id %d", id);
		if (verbose) {
			std::cout << "copy file " << id << std::endl;
		}
		if (!dryrun) {
			astro::image::ImagePtr	imageptr
				= localrepository->getImage(id);
			remoterepository->save(convertfile(imageptr));
		}
	}
	virtual void	pull(int id) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "copy image id %d", id);
		if (verbose) {
			std::cout << "pulling " << id << std::endl;
		}
		if (!dryrun) {
			ImageFile imagefile = remoterepository->getImage(id);
			localrepository->save(convertfile(imagefile));
		}
	}
};

/**
 * \brief Replicator to replicate between two remote repositories
 *
 * The first remote repository is called the local repository although
 * it is remote image repository.
 */
class RemoteRepoReplicator : public BaseRepoReplicator {
protected:
	RepositoriesPrx	localrepositories;
	RepositoryPrx	localrepository;
public:
	RemoteRepoReplicator(const astro::URL& localurl,
		const astro::URL& remoteurl)
		: BaseRepoReplicator(remoteurl) {
		Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
		Ice::ObjectPrx	base = ic->stringToProxy(
					localurl.connect("Repositories"));
		localrepositories = RepositoriesPrx::checkedCast(base);
		if (!localrepositories) {
			throw std::runtime_error("no repositories proxy");
		}
		localrepository = localrepositories->get(localurl.path());
		if (!remoterepository) {
			throw std::runtime_error("no repository proxy");
		}
		localuuids = getUUIDs(localrepository);
	}
	virtual std::set<int>	getlocalids(const std::list<std::string>& tocopy) const {
		std::set<int>	copyids;
		for (auto ptr = tocopy.begin(); ptr != tocopy.end(); ptr++) {
			copyids.insert(localrepository->getId(*ptr));
		}
		return copyids;
	}
	virtual void	push(int id) {	
		debug(LOG_DEBUG, DEBUG_LOG, 0, "copy image id %d", id);
		if (verbose) {
			std::cout << "pushing " << id << std::endl;
		}
		if (!dryrun) {
			ImageFile imagefile = localrepository->getImage(id);
			remoterepository->save(imagefile);
		}
	}
	virtual void	pull(int id) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "copy image id %d", id);
		if (verbose) {
			std::cout << "pulling " << id << std::endl;
		}
		if (!dryrun) {
			ImageFile imagefile = remoterepository->getImage(id);
			localrepository->save(imagefile);
		}
	}
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
		return command_help(argv[0]);
	}

	// all other commands need a remote Repositories reference
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	astro::ServerName	servername(server);

	// list command needs nothing more
	if (command == "list") {
		return command_list(servername);
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

	// if the remote repository is a URL, then we use it to set the
	// server name (instead of the --server option
	astro::URL	remoteurl("repo:");
	try {
		astro::URL	u(remotereponame);
		remoteurl = u;
	} catch (...) {
		remoteurl.host(servername.host());
		remoteurl.port(servername.port());
		remoteurl[0] = remotereponame;
	}

	// create the replicator
	try {
		astro::URL	localurl(localreponame);
		RemoteRepoReplicator	replicator(localreponame, remoteurl);
		return replicator.command(command);
	} catch (...) {
		LocalRepoReplicator	replicator(localreponame, remoteurl);
		return replicator.command(command);
	}

	return EXIT_FAILURE;
}

} // namespace snowrepo
} // namespace app
} // namespace snowstar

int main(int argc, char *argv[]) {
	return astro::main_function<snowstar::app::snowrepo::main>(argc, argv);
}
