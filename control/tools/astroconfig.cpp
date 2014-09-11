/*
 * astroconfig.cpp -- manage the configuration
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConfig.h>
#include <cstdlib>
#include <cstdio>
#include <stdexcept>
#include <typeinfo>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroUtils.h>
#include <includes.h>
#include <algorithm>
#include <stacktrace.h>

using namespace astro::config;
using namespace astro::project;

namespace astro {

/**
 * \brief Table of options
 */
static struct option	longopts[] = {
/* name         argument?               int*    	int */
{ "config",	required_argument,	NULL,		'c' }, /* 0 */
{ "debug",	no_argument,		NULL,		'd' }, /* 1 */
{ "help",	no_argument,		NULL,		'h' }, /* 2 */
{ NULL,		0,			NULL,		0   }
};

/**
 * \brief usage message
 */
void	usage(const char *progname) {
	std::cerr << "usage:" << std::endl;
	std::cerr << progname << " [ options ] { get | set | delete } domain section name [ value ]" << std::endl;
	std::cerr << "Get, set or delete configuration variables in domain (currently only 'global'";
	std::cerr << "is valid), identified by the section and the name.";
	std::cerr << std::endl;
	std::cerr << progname << " [ options ] imagerepo list" << std::endl;
	std::cerr << progname << " [ options ] imagerepo add <reponame> <director>";
	std::cerr << std::endl;
	std::cerr << progname << " [ options ] imagerepo remove <reponame>";
	std::cerr << std::endl;
	std::cerr << "list, add or delete image repositores" << std::endl;
	std::cerr << progname << " [ options ] project list";
	std::cerr << std::endl;
	std::cerr << progname << " [ options ] project add <projname> ...";
	std::cerr << std::endl;
	std::cerr << progname << " [ options ] project show <projname>";
	std::cerr << std::endl;
	std::cerr << progname << " [ options ] project remove <projname>";
	std::cerr << std::endl;
	std::cerr << "list, add or remove projects, show project details";
	std::cerr << std::endl;
	std::cerr << "options:" << std::endl;
	std::cerr << "  -c,--config=<configfile>     use configuration from <configfile>" << std::endl;
	std::cerr << "  -d,--debug                   increase debug level";
	std::cerr << std::endl;
	std::cerr << "  -h,--help                    show this help message";
	std::cerr << std::endl;
}

/**
 * \brief set a global configuration variable
 */
int	command_set_global(const std::vector<std::string>& arguments) {
	if (arguments.size() < 5) {
		std::cerr << "not enough arguments for set command";
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}
	ConfigurationPtr	configuration = Configuration::get();
	configuration->setglobal(arguments[2], arguments[3], arguments[4]);
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the set command
 */
int	command_set(const std::vector<std::string>& arguments) {
	if (arguments.size() < 2) {
		std::cerr << "not enough arguments for get command";
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}
	std::string	domain = arguments[1];
	if (domain == "global") {
		return command_set_global(arguments);
	}
	std::cerr << "domain " << domain << "  not implemented" << std::endl;
	return EXIT_FAILURE;
}

/**
 * \brief Implementation of the get global command
 */
int	command_get_global(const std::vector<std::string>& arguments) {
	if (arguments.size() < 4) {
		std::cerr << "not enough arguments for get command";
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}
	try {
		ConfigurationPtr	configuration = Configuration::get();
		std::cout << configuration->global(arguments[2],
					arguments[3]); 
		std::cout << std::endl;
		return EXIT_SUCCESS;
	} catch (const std::exception& x) {
		std::cerr << "not found: "  << x.what() << std::endl;
	}
	return EXIT_FAILURE;
}

/**
 * \brief Implementation of the get command
 */
int	command_get(const std::vector<std::string>& arguments) {
	if (arguments.size() < 2) {
		std::cerr << "not enough arguments for get command";
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}
	std::string	domain = arguments[1];
	if (domain == "global") {
		return command_get_global(arguments);
	}
	std::cerr << "domain " << domain << " not implemented" << std::endl;
	return EXIT_FAILURE;
}

/**
 * \brief Implementation of the global delete command
 */
int	command_delete_global(const std::vector<std::string>& arguments) {
	if (arguments.size() < 4) {
		std::cerr << "not enough arguments for delete command";
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}
	try {
		ConfigurationPtr	configuration = Configuration::get();
		configuration->removeglobal(arguments[2], arguments[3]); 
		return EXIT_SUCCESS;
	} catch (const std::exception& x) {
		std::cerr << "not found: "  << x.what() << std::endl;
	}
	return EXIT_FAILURE;
}

/**
 * \brief Implementation of the delete command
 */
int	command_delete(const std::vector<std::string>& arguments) {
	if (arguments.size() < 2) {
		std::cerr << "not enough arguments for get command";
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}
	std::string	domain = arguments[1];
	if (domain == "global") {
		return command_delete_global(arguments);
	}
	std::cerr << "domain " << domain << " not implemented" << std::endl;
	return EXIT_FAILURE;
}

/**
 * \brief Implementation of the list global command
 */
int	command_list_global(const std::vector<std::string>& /* arguments */) {
	std::list<ConfigurationEntry>	entries
		= Configuration::get()->globallist();
	std::for_each(entries.begin(), entries.end(),
		[](ConfigurationEntry entry) {
			std::cout << entry.section;
			std::cout << "\t";
			std::cout << entry.name;
			std::cout << "\t";
			std::cout << entry.value;
			std::cout << "\n";
		}
	);
	return EXIT_SUCCESS;
}

/**
 * \brief List all repositories
 */
int	list_repo() {
	std::list<ImageRepoInfo>	repoinfolist
		= Configuration::get()->listrepo();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d ImageRepoInfo objects",
		repoinfolist.size());
	std::for_each(repoinfolist.begin(), repoinfolist.end(),
		[](const ImageRepoInfo& repoinfo) {
			std::cout << stringprintf("%-8.8s %s %s\n",
				repoinfo.reponame.c_str(),
				repoinfo.database.c_str(),
				repoinfo.directory.c_str());
		}
	);
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the image repository commands
 */
int	command_imagerepo(const std::vector<std::string>& arguments) {
	if (arguments.size() < 2) {
		std::cerr << "no image repo sub command" << std::endl;
		return EXIT_FAILURE;
	}
	ConfigurationPtr	configuration = Configuration::get();
	if (arguments[1] == "add") {
		if (arguments.size() < 4) {
			std::cerr << "not enough arguments for add command";
			std::cerr << std::endl;
			return EXIT_FAILURE;
		}
		configuration->addrepo(arguments[2], arguments[3]);
		return EXIT_SUCCESS;
	}
	if (arguments[1] == "list") {
		return list_repo();
	}
	if (arguments[1] == "remove") {
		configuration->removerepo(arguments[2]);
		return EXIT_SUCCESS;
	}
	std::cerr << "unknown subcommand " << arguments[1] << std::endl;
	return EXIT_FAILURE;
}

/**
 * \brief Implementation of the project command
 */
int	command_project(const std::vector<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "project command");
	if (arguments.size() < 2) {
		throw std::runtime_error("not enough arguments for project command");
	}
	std::string	subcommand = arguments[1];
	if (subcommand == "list") {
		// list projects
		std::list<Project>	projects
			= Configuration::get()->listprojects();
		if (projects.size() == 0) {
			return EXIT_SUCCESS;
		}
		std::cout << "started  project         repository  description";
		std::cout << std::endl;
		std::for_each(projects.begin(), projects.end(),
			[](const Project& project) {
				std::cout << timeformat("%d.%m.%y ",
					project.started());
				std::cout << stringprintf("%-16.16s",
					project.name().c_str());
				std::cout << stringprintf("%-11.11s ",
					project.repository().c_str());
				std::cout << project.description();
				std::cout << std::endl;
			}
		);
		return EXIT_SUCCESS;
	}
	std::string	projectname = arguments[2];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "project name = %s",
		projectname.c_str());

	// there must be more arguments
	if (arguments.size() < 3) {
		throw std::runtime_error("not enough arguments");
		return EXIT_FAILURE;
	}
	if (subcommand == "add") {
		Project	project;
		project.name(projectname);
		AttributeValuePairs	av(arguments, 3);
		if (av.has("description")) {
			project.description(av("description"));
		}
		if (av.has("repository")) {
			project.repository(av("repository"));
		}
		if (av.has("object")) {
			project.object(av("object"));
		}
		Configuration::get()->addproject(project);
		return EXIT_SUCCESS;
	}
	if (subcommand == "show") {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "show project '%s'",
			projectname.c_str());
		Project	project = Configuration::get()->project(projectname);
		std::cout << "name:         ";
		std::cout << project.name() << std::endl;
		std::cout << "description:  ";
		std::cout << project.description() << std::endl;
		std::cout << "object:       ";
		std::cout << project.object() << std::endl;
		std::cout << "repository:   ";
		std::cout << project.repository() << std::endl;
		std::cout << "started:      ";
		std::cout << timeformat("%Y-%m-%d %H:%M:%S", project.started());
		std::cout << std::endl;
		return EXIT_SUCCESS;
	}
	if (subcommand == "remove") {
		Configuration::get()->removeproject(projectname);
		return EXIT_SUCCESS;
	}

	// if we get to this point, then we have an unknown command
	std::cerr << "unknown project subcommand " << subcommand << std::endl;
	return EXIT_FAILURE;
}

/**
 * \brief 
 */
int	command_list(const std::vector<std::string>& arguments) {
	if (arguments.size() < 2) {
		std::cerr << "not enough arguments for get command";
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}
	std::string	domain = arguments[1];
	if (domain == "global") {
		return command_list_global(arguments);
	}
	std::cerr << "domain " << domain << " not implemented" << std::endl;
	return EXIT_FAILURE;
}

/**
 * \brief main method of the astroconfig program
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

	ConfigurationPtr	configuration = Configuration::get();

	// remaining arguments are 
	std::vector<std::string>	arguments;
	while (optind < argc) {
		arguments.push_back(std::string(argv[optind++]));
	}

	if (0 == arguments.size()) {
		std::cerr << "not engough arguments" << std::endl;
		return EXIT_FAILURE;
	}

	std::string	verb = arguments[0];
	
	if (verb == "get") {
		return command_get(arguments);
	}
	if (verb == "set") {
		return command_set(arguments);
	}
	if (verb == "delete") {
		return command_delete(arguments);
	}
	if (verb == "list") {
		return command_list(arguments);
	}
	if (verb == "imagerepo") {
		return command_imagerepo(arguments);
	}
	if (verb == "project") {
		return command_project(arguments);
	}
	
	std::cerr << "command " << verb << " not implemented" << std::endl;
	return EXIT_FAILURE;
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
