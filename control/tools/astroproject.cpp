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

using namespace astro;
using namespace astro::config;
using namespace astro::project;

namespace astro {
namespace app {
namespace project {

bool	verbose = false;

/**
 * \brief Table of options
 */
static struct option	longopts[] = {
/* name         argument?               int*    	int */
{ "config",	required_argument,	NULL,		'c' }, /* 0 */
{ "debug",	no_argument,		NULL,		'd' }, /* 1 */
{ "help",	no_argument,		NULL,		'h' }, /* 2 */
{ "verbose",	no_argument,		NULL,		'v' }, /* 3 */
{ NULL,		0,			NULL,		0   }
};

/**
 * \brief usage message
 */
void	usage(const char *progname) {
	Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] help" << std::endl;
	std::cout << std::endl;
	std::cout << "display a help message about the astroproject command";
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] list" << std::endl;
	std::cout << p << " [ options ] add <projname> attributes ..." << std::endl;
	std::cout << p << " [ options ] show <projname>" << std::endl;
	std::cout << p << " [ options ] remove <projname>" << std::endl;
	std::cout << std::endl;
	std::cout << "list, add or remove projects, show project details";
	std::cout << std::endl;
	std::cout << "attributes of a project created with the add command are "
		"to be specified as" << std::endl;
	std::cout << "attribte=value pairs:" << std::endl;
	std::cout << std::endl;
	std::cout << "    description=<description>" << std::endl;
	std::cout << "    object=<object>" << std::endl;
	std::cout << "    repository=<repo>" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] <proj> list" << std::endl;
	std::cout << p << " [ options ] <proj> add number attributes ..." << std::endl;
	std::cout << p << " [ options ] <proj> copy number newnumber" << std::endl;
	std::cout << p << " [ options ] <proj> show number" << std::endl;
	std::cout << p << " [ options ] <proj> remove number" << std::endl;
	std::cout << std::endl;
	std::cout << "list, add, show and remove parts for a project"
		<< std::endl;
	std::cout << std::endl;
	std::cout << "attributes for a part in the add command are specified "
		"as follows:" << std::endl;
	std::cout << std::endl;
	std::cout << "    temperature=<temperature>" << std::endl;
	std::cout << "    filter=<filtername>" << std::endl;
	std::cout << "    instrument=<instrument>" << std::endl;
	std::cout << "    taskserver=<server:port>" << std::endl;
	std::cout << "    frame=widthxheight@xoffset,yoffset" << std::endl;
	std::cout << "    exposuretime=<time>" << std::endl;
	std::cout << "    gain=<gain>" << std::endl;
	std::cout << "    limit=<limit>" << std::endl;
	std::cout << "    binning=XxY" << std::endl;
	std::cout << "    shutter=<open|closed>" << std::endl;
	std::cout << "    purpose=<light|dark|flat>" << std::endl;

	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -c,--config=<configfile>     use configuration from <configfile>" << std::endl;
	std::cout << "  -d,--debug                   increase debug level";
	std::cout << std::endl;
	std::cout << "  -h,--help                    show this help message";
	std::cout << std::endl;
	std::cout << "  -v,--verbose                 verbose display (mostly for list command)";
	std::cout << std::endl;
}

/**
 * \brief help command
 */
int	command_help(const std::list<std::string>& /* arguments */) {
	usage("astroproject");
	return EXIT_SUCCESS;
}

/**
 * \brief list all the projects
 */
int	command_list() {
	// list projects
	ConfigurationPtr	config = Configuration::get();
	ProjectConfigurationPtr	projectconfig = ProjectConfiguration::get(config);
	std::list<Project>	projects = projectconfig->listprojects();
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

/**
 * \brief add a new project
 */
int	command_add(const std::string& projectname,
		const std::list<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "project name = %s",
		projectname.c_str());
	Project	project(projectname);
	AttributeValuePairs	av(arguments);
	if (av.has("description")) {
		project.description(av("description"));
		av.erase("description");
	}
	if (av.has("repository")) {
		project.repository(av("repository"));
		av.erase("repository");
	}
	if (av.has("object")) {
		project.object(av("object"));
		av.erase("object");
	}
	std::set<std::string>	s = av.attributes();
	if (s.size() > 0) {
		std::cerr << "unknown attributes: ";
		std::for_each(s.begin(), s.end(),
			[](const std::string& s) {
				std::cerr << " ";
				std::cerr << s;
			}
		);
		return EXIT_FAILURE;
	}
	ConfigurationPtr	config = Configuration::get();
	ProjectConfigurationPtr	projects = ProjectConfiguration::get(config);
	projects->addproject(project);
	return EXIT_SUCCESS;
}

/**
 * \brief Show the defintion of a project
 */
int	command_show(const std::string& projectname,
		const std::list<std::string>& /* arguments */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "show project '%s'",
		projectname.c_str());
	ConfigurationPtr	config = Configuration::get();
	ProjectConfigurationPtr	projects = ProjectConfiguration::get(config);
	Project	project = projects->project(projectname);
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

/**
 * \brief remove a project from database
 */
int	command_remove(const std::string& projectname,
		const std::list<std::string>& /* arguments */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "project name = %s",
		projectname.c_str());
	ConfigurationPtr	config = Configuration::get();
	ProjectConfigurationPtr	projects = ProjectConfiguration::get(config);
	projects->removeproject(projectname);
	return EXIT_SUCCESS;
}

/**
 * \brief List the parts of a project
 */
int	command_partlist(const std::string& projectname,
		const std::list<std::string>& /* arguments */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "list parts of project %s",
		projectname.c_str());
	ConfigurationPtr	config = Configuration::get();
	ProjectConfigurationPtr	projects = ProjectConfiguration::get(config);
	std::list<astro::project::PartPtr>	parts
		= projects->listparts(projectname);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "project has %d parts", parts.size());
	if (parts.size() == 0) {
		std::cerr << "no parts" << std::endl;
		return EXIT_SUCCESS;
	}
	std::cout << "part instrument ";
	if (verbose) {
		std::cout << "rectangle           bin      exp  gain limit   temp purpose filter     taskserver         ";
	} else {
		std::cout << "size        exp  temp purpose filter taskserver    ";
	}
	std::cout << "taskid repoid"
		<< std::endl;
	for (auto ptr = parts.begin(); ptr != parts.end(); ptr++) {
		std::cout << stringprintf("%04d ", (*ptr)->partno());
		std::cout << stringprintf("%-10.10s ",
			(*ptr)->instrument().c_str());
		astro::camera::Exposure	e = (*ptr)->exposure();
		if (verbose) {
			std::cout << stringprintf("%-18.18s ",
				e.frame().toString().c_str());
			std::cout << stringprintf(" %-5.5s",
					e.mode().toString().c_str());
			int	lt = floor(log10(e.exposuretime()));
			if (lt < 0) { lt = 0; }
			lt = 3 - lt;
			if (lt < 0) { lt = 0; }
			std::cout << stringprintf("%7.*f", lt, e.exposuretime());
			lt = ceil(2 - log10(e.gain()));
			if (lt < 0) { lt = 0; }
			std::cout << stringprintf("%6.*f", lt, e.gain());
			std::cout << stringprintf("%6.0f", e.limit());
			std::cout << stringprintf("%7.1f", (*ptr)->temperature());
		} else {
			std::cout << stringprintf("%-10.10s",
				e.frame().size().toString().c_str());
			std::cout << stringprintf("%5.0f", e.exposuretime());
			std::cout << stringprintf("%6.1f", (*ptr)->temperature());
		}
		std::cout << stringprintf(" %-7.7s",
			astro::camera::Exposure::purpose2string(e.purpose()).c_str());
		if (verbose) {
			std::cout << stringprintf(" %-10.10s",
				(*ptr)->filtername().c_str());
			std::cout << stringprintf(" %-18.18s",
				(*ptr)->taskserver().c_str());
		} else {
			std::cout << stringprintf(" %-6.6s",
				(*ptr)->filtername().c_str());
			std::cout << stringprintf(" %-13.13s",
				(*ptr)->taskserver().c_str());
		}
		if ((*ptr)->taskid() >= 0) {
			std::cout << stringprintf(" %6ld",
				(*ptr)->taskid());
		} else {
			std::cout << "      ?";
		}
		if ((*ptr)->repoid() >= 0) {
			std::cout << stringprintf(" %6ld",
				(*ptr)->repoid());
		} else {
			std::cout << "      ?";
		}
	
		std::cout << std::endl;
	}
	return EXIT_SUCCESS;
}

/**
 * \brief Add a part to the project
 */
int	command_partadd(const std::string& projectname, long partno,
		const std::list<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add part %ld to project %s", partno,
		projectname.c_str());
	AttributeValuePairs	av(arguments);
	astro::project::Part	part;
	part.partno(partno);

	if (av.has("temperature")) {
		part.temperature(std::stod(av("temperature")));
		av.erase("temperature");
	}

	if (av.has("filter")) {
		part.filtername(av("filter"));
		av.erase("filter");
	}

	if (av.has("instrument")) {
		part.instrument(av("instrument"));
		av.erase("instrument");
	}

	if (av.has("taskserver")) {
		part.taskserver(av("taskserver"));
		av.erase("taskserver");
	}

	astro::camera::Exposure	exposure;

	if (av.has("frame")) {
		exposure.frame(astro::image::ImageRectangle(av("frame")));
		av.erase("frame");
	}
	if (av.has("exposuretime")) {
		exposure.exposuretime(std::stod(av("exposuretime")));
		av.erase("exposuretime");
	}
	if (av.has("gain")) {
		exposure.gain(std::stod(av("gain")));
		av.erase("gain");
	}
	if (av.has("limit")) {
		exposure.limit(std::stod(av("limit")));
		av.erase("limit");
	}
	if (av.has("binning")) {
		exposure.mode(astro::camera::Binning(av("binning")));
		av.erase("binning");
	}
	if (av.has("shutter")) {
		exposure.shutter(astro::camera::Shutter::string2state(av("shutter")));
		av.erase("shutter");
	}
	if (av.has("purpose")) {
		exposure.purpose(astro::camera::Exposure::string2purpose(av("purpose")));
		av.erase("purpose");
	}

	part.exposure(exposure);

	std::set<std::string>	s = av.attributes();
	if (s.size() > 0) {
		std::cerr << "unused attributes: ";
		std::for_each(s.begin(), s.end(),
			[](const std::string& s) {
				std::cerr << " ";
				std::cerr << s;
			}
		);
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}

	// add the project part to the database
	ConfigurationPtr	config = Configuration::get();
	ProjectConfigurationPtr	projects = ProjectConfiguration::get(config);
	projects->addpart(projectname, part);
	return EXIT_SUCCESS;
}

/**
 * \brief Show details about a part
 */
int	command_partshow(const std::string& projectname, long partno) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "show part %ld to project %s", partno,
		projectname.c_str());
	ConfigurationPtr	config = Configuration::get();
	ProjectConfigurationPtr	projects = ProjectConfiguration::get(config);
	PartPtr	part = projects->part(projectname, partno);
	std::cout << "No:           " << part->partno() << std::endl;
	std::cout << "Filtername:   " << part->filtername() << std::endl;
	return EXIT_SUCCESS;
}

/**
 * \brief Copy a part
 */
int	command_partcopy(const std::string& projectname, long partno,
		const std::list<long>& partnos) {
	if (partnos.size() == 0) {
		std::cerr << "missing new part number" << std::endl;
		return EXIT_FAILURE;
	}
	ConfigurationPtr	config = Configuration::get();
	ProjectConfigurationPtr	projects = ProjectConfiguration::get(config);
	PartPtr	part = projects->part(projectname, partno);
	for (auto ptr = partnos.begin(); ptr != partnos.end(); ptr++) {
		long	newpartno = *ptr;
		part->partno(newpartno);
		projects->addpart(projectname, *part);
	}
	return EXIT_SUCCESS;
}

/**
 * \brief Remove a part from the project
 */
int	command_partremove(const std::string& projectname,
		const std::list<long>& partnos) {
	ConfigurationPtr	config = Configuration::get();
	ProjectConfigurationPtr	projects = ProjectConfiguration::get(config);
	for (auto ptr = partnos.begin(); ptr != partnos.end(); ptr++) {
		long	partno = *ptr;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "remove part %ld to project %s",
			partno, projectname.c_str());
		projects->removepart(projectname, partno);
	}
	return EXIT_SUCCESS;
}

/**
 * \brief main method of the astroconfig program
 */
int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "c:dh", longopts,
		&longindex))) {
		switch (c) {
		case 'c':
			Configuration::set_default(std::string(optarg));
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

	ConfigurationPtr	configuration = Configuration::get();

	// remaining arguments are 
	std::list<std::string>	arguments;
	while (optind < argc) {
		arguments.push_back(std::string(argv[optind++]));
	}

	if (0 == arguments.size()) {
		std::cerr << "not engough arguments" << std::endl;
		return EXIT_FAILURE;
	}

	// get the first element from the argument list
	std::string	verb = arguments.front();
	arguments.pop_front();

	if (verb == "help") {
		return command_help(arguments);
	}

	// project related commands
	if (verb == "list") {
		return command_list();
	}
	std::string	projectname = arguments.front();
	if (verb == "add") {
		return command_add(projectname, arguments);
	}
	if (verb == "show") {
		return command_show(projectname, arguments);
	}
	if (verb == "remove") {
		return command_remove(projectname, arguments);
	}

	// if we get to this point, then we have a part related command,
	// so the verb argument is rather a project name
	projectname = verb;
	if (0 == arguments.size()) {
		std::cerr << "missing part command" << std::endl;
		return EXIT_FAILURE;
	}
	verb = arguments.front();
	arguments.pop_front();

	if (verb == "list") {
		return command_partlist(projectname, arguments);
	}
	if (0 == arguments.size()) {
		std::cerr << "missing part number" << std::endl;
		return EXIT_FAILURE;
	}
	long	partno = std::stol(arguments.front());
	arguments.pop_front();
	if (verb == "add") {
		return command_partadd(projectname, partno, arguments);
	}
	std::list<long>	partnos;
	partnos.push_back(partno);
	while (arguments.size()) {
		partnos.push_back(std::stol(arguments.front()));
		arguments.pop_front();
	}
	if (verb == "copy") {
		partnos.pop_front();
		return command_partcopy(projectname, partno, partnos);
	}
	if (verb == "show") {
		return command_partshow(projectname, partno);
	}
	if (verb == "remove") {
		return command_partremove(projectname, partnos);
	}

	std::cerr << "command " << verb << " not implemented" << std::endl;
	return EXIT_FAILURE;
}

} // namespace project
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::project::main>(argc, argv);
}
