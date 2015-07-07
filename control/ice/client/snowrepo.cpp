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
#include <AstroIO.h>
#include <repository.h>
#include <IceConversions.h>
#include <RepoReplicators.h>

using namespace snowstar;

namespace snowstar {
namespace app {
namespace snowrepo {

std::string	server("localhost");
std::string	project("");
bool	dryrun = false;
bool	verbose = false;

void	usage(const char *progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] help" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] list [ reponame ]" << std::endl;
	std::cout << std::endl;
	std::cout << "List names of repositories known on a repository server, "
		"list contents of a " << std::endl;
	std::cout << "remote repo." << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] { push | pull | synchronize } "
		"<localrepo> <remoterepo>" << std::endl;
	std::cout << std::endl;
	std::cout << "replicate files from a local repository to a remote "
		"repository" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] add reponame images ..." << std::endl;
	std::cout << p << " [ options ] get reponame id filename" << std::endl;
	std::cout << p << " [ options ] remove reponame id ..." << std::endl;
	std::cout << std::endl;
	std::cout << "Add images to a remote repository, retrieve an image or "
		"delete images from a" << std::endl;
	std::cout << "remote repository." << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -c,--config=<cfg>       use configuration <cfg>" << std::endl;
	std::cout << " -d,--debug              increase debug level" << std::endl;
	std::cout << " -n,--dry-run            don't do anything, just report on what would be done" << std::endl;
	std::cout << " -p,--project=<project>  only replicate images of some project" << std::endl;
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
	std::cout << "The snowrepo program also allows to add, retrieve or "
		"delete individual images" << std::endl;
	std::cout << "to or from the image repository. This extends the "
		"functionality of the local" << std::endl;
	std::cout << "imagerepo(1) program" << std::endl;
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

/**
 * \brief Auxiliary function to get a remote repository proxy
 */
static RepositoryPrx	getRemoteRepo(const astro::ServerName& servername,
				const std::string& reponame) {
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	Ice::ObjectPrx	base = ic->stringToProxy(
					servername.connect("Repositories"));
	RepositoriesPrx	remoterepositories = RepositoriesPrx::checkedCast(base);
	if (!remoterepositories) {
		throw std::runtime_error("no repositories proxy");
	}
	RepositoryPrx	repository;
	repository = remoterepositories->get(reponame);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got remote repository %s",
		reponame.c_str());
	return repository;
}

/**
 * \brief List the contents of a repository
 */
int	command_list(const astro::ServerName& servername,
		const std::string& reponame) {
	RepositoryPrx	repository = getRemoteRepo(servername, reponame);

	// retrieve a list of uuids from the remote server
	std::string	condition("0 = 0");
	if (project.size() > 0) {
		condition = astro::stringprintf("project = '%s'",
			project.c_str());
	}
	uuidlist u = repository->getUUIDsCondition(condition);
	std::set<int>	ids;
	for (auto ptr = u.begin(); ptr != u.end(); ptr++) {
		ids.insert(repository->getId(*ptr));
	}
	std::cout << "id   instrument size      purpose bin  exp  temp "
		"observation    project ";
	if (verbose) {
		std::cout << " uuid                                 filename";
	}
	std::cout << std::endl;
	for (auto ptr = ids.begin(); ptr != ids.end(); ptr++) {
		int	id = *ptr;
		ImageInfo	info = repository->getInfo(id);
		std::cout << astro::stringprintf("%04d %-10.10s %-9.9s ", id,
			info.instrument.c_str(),
			convert(info.size).toString().c_str());
		std::cout << astro::stringprintf("%-8.8s",
			info.purpose.c_str());
		std::cout << astro::stringprintf("%1dx%1d%5.1f %5.1f ",
			info.binning.x, info.binning.y,
			info.exposuretime,
			info.temperature);
		std::cout << astro::timeformat("%d.%m.%y %H:%M ",
                                converttime(info.observationago));
		std::cout << astro::stringprintf("%-8.8s",
			info.project.c_str());
		if (verbose) {
			std::cout << astro::stringprintf(" %-36.36s %s",
				info.uuid.c_str(),
				info.filename.c_str());
		}
		std::cout << std::endl;
	}
	
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the add command
 *
 * This command adds images identified by filename to the remote repository
 */
int	command_add(const astro::ServerName& servername,
		const std::string& reponame,
		const std::list<std::string>& filenames) {
	// get the repo
	RepositoryPrx	repository = getRemoteRepo(servername, reponame);

	// go through the images
	for (auto ptr = filenames.begin(); ptr != filenames.end(); ptr++) {
		astro::io::FITSin	in(*ptr);
		astro::image::ImagePtr	imageptr = in.read();
		repository->save(convertfile(imageptr));
	}
	
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the get command
 *
 * This command retrieves an image identified by id from a remote repository,
 * and saves it as a local file.
 */
int	command_get(const astro::ServerName& servername,
		const std::string& reponame, int id,
		const std::string& filename) {
	// get the repo
	RepositoryPrx	repository = getRemoteRepo(servername, reponame);

	ImageFile	image = repository->getImage(id);
	astro::io::FITSout	out(filename);
	out.write(convertfile(image));
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the remove command
 *
 * This command removes images identified by a list of ids from a remote
 * repository.
 */
int	command_remove(const astro::ServerName& servername,
		const std::string& reponame, const std::list<int>& ids) {
	// get the repo
	RepositoryPrx	repository = getRemoteRepo(servername, reponame);
	for (auto ptr = ids.begin(); ptr != ids.end(); ptr++) {
		if (dryrun) {
			std::cout << "remove " << *ptr << std::endl;
		} else {
			repository->remove(*ptr);
		}
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
	while (EOF != (c = getopt_long(argc, argv, "c:dhp:s:v",
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
		case 'p':
			project = optarg;
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
		if (argc <= optind) {
			return command_list(servername);
		}
		std::string	reponame = argv[optind++];
		return command_list(servername, reponame);
	}

	// remaining commands need at least a local repository name
	if (argc <= optind) {
		throw std::runtime_error("local repository name missing");
	}
	std::string	localreponame = argv[optind++];

	// add command uses remaining arguments as file names
	if (command == "add") {
		std::list<std::string>	filenames;
		while (optind < argc) {
			filenames.push_back(argv[optind++]);
		}
		return command_add(servername, localreponame, filenames);
	}

	// get command uses two more arguments: 
	if (command == "get") {
		if (optind >= argc) {
			throw std::runtime_error("id argument missing");
		}
		int	id = std::stoi(argv[optind++]);
		if (optind >= argc) {
			throw std::runtime_error("file name argument missing");
		}
		std::string	filename(argv[optind++]);
		return command_get(servername, localreponame, id, filename);
	}

	// the remove command uses the remaining arguments as ids
	if (command == "remove") {
		std::list<int>	ids;
		while (optind < argc) {
			ids.push_back(std::stoi(argv[optind++]));
		}
		return command_remove(servername, localreponame, ids);
	}

	// get the remote repository name
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
		RemoteRepoReplicator	replicator(localreponame, remoteurl,
						project);
		replicator.verbose(verbose);
		replicator.dryrun(dryrun);
		return replicator.command(command);
	} catch (...) {
		LocalRepoReplicator	replicator(localreponame, remoteurl,
						project);
		replicator.verbose(verbose);
		replicator.dryrun(dryrun);
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