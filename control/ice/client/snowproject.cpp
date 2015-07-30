/*
 * snowproject.cpp -- submit a project part as a task to the task server
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <CommunicatorSingleton.h>
#include <includes.h>
#include <AstroConfig.h>
#include <tasks.h>
#include <IceConversions.h>
#include <CommonClientTasks.h>
#include <AstroFormat.h>
#include <AstroConfig.h>
#include <AstroProject.h>
#include <AstroIO.h>

namespace snowstar {
namespace app {
namespace snowproject {

bool	verbose = false;

/**
 * \brief Usage function for the snowtask program
 */
void	usage(const char *progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] help" << std::endl;
	std::cout << p << " [ options ] submit projectname partno" << std::endl;
	std::cout << p << " [ options ] image projectname partno" << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -c,--config=<cfg>  use configuration from a cfg"
		<< std::endl;
	std::cout << " -d,--debug         increase debug level" << std::endl;
	std::cout << " -h,--help          show this help and exit" << std::endl;
	std::cout << " -v,--verbose       verbose mode" << std::endl;
}

/**
 * \brief Display help about this program
 */
int	command_help(const char *progname) {
	usage(progname);
	return EXIT_SUCCESS;
}

/**
 * \brief implementation of the submit command, part specific version
 */
int	command_submit(const std::string& projectname,
		astro::project::PartPtr part) {
	// get the parameters for the part
	astro::ServerName	servername(part->taskserver());

	// get the Tasks interface
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	Ice::ObjectPrx  base = ic->stringToProxy(servername.connect("Tasks"));
	TaskQueuePrx	tasks = TaskQueuePrx::checkedCast(base);

	// check whether the task id exists
	if (part->taskid() > 0) {
		try {
			TaskInfo info = tasks->info(part->taskid());
		} catch (NotFound) {
		} catch (...) {
		}
	}

	// get configuration information
	astro::config::ConfigurationPtr config
		= astro::config::Configuration::get();
	astro::config::InstrumentConfigurationPtr	instruments
		= astro::config::InstrumentConfiguration::get(config);
	astro::config::InstrumentPtr    instrument
		= instruments->instrument(part->instrument());

	// prepare the parameters 
	TaskParameters  parameters;

	// get the device information from the instrument
	parameters.camera = instrument->component(
		astro::DeviceName::Camera)->name();
	parameters.ccdid = instrument->component(
		astro::DeviceName::Ccd)->unit();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera: %s, ccd: %d",
		parameters.camera.c_str(), parameters.ccdid);

	// get the temperature from the 
	parameters.ccdtemperature = part->temperature() + 273.15;

	// filterwheel parameters
	parameters.filterwheel = "";
	try {
		parameters.filterwheel = instrument->devicename(
			astro::DeviceName::Filterwheel).toString();
	} catch (...) {
	}
	parameters.filter = part->filtername();

	// exposure parameters
	parameters.exp = convert(part->exposure());

	// everything is ready now, submit the task
	int     taskid = tasks->submit(parameters);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "submitted new task %d", taskid);

	// add the task id to the part
	astro::config::ProjectConfigurationPtr	projects
		= astro::config::ProjectConfiguration::get(config);
	projects->parttask(projectname, part->partno(), taskid);
	return EXIT_SUCCESS;
}

/**
 * \brief implementation of the submit command, global version
 */
int	command_submit(const astro::project::Project& project,
		const std::list<long>& partnos) {
	for (auto partnoptr = partnos.begin(); partnoptr != partnos.end();
		partnoptr++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "working on part %ld",
			*partnoptr);
		astro::project::PartPtr	part = project.part(*partnoptr);

		// submit the part
		command_submit(project.name(), part);
	}
	return EXIT_SUCCESS;
}

/**
 * \brief implementation of the image command, part specific version
 */
int	command_image(const astro::project::Project& project,
		astro::project::PartPtr partptr) {
	// get the configuration
	astro::config::ConfigurationPtr	config
		= astro::config::Configuration::get();

	// do we really have a task id?
	if (partptr->taskid() < 0) {
		std::cerr << "task " << partptr->partno() << " has no task";
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}

	// get the repository
	std::string	reponame = project.repository();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image repository: %s",
		reponame.c_str());
	astro::config::ImageRepoConfigurationPtr	imagerepos
		= astro::config::ImageRepoConfiguration::get(config);
	astro::project::ImageRepoPtr	repository = imagerepos->repo(reponame);

	// get the parameters for the part
	astro::ServerName	servername(partptr->taskserver());

	// get the Tasks interface
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	Ice::ObjectPrx  base = ic->stringToProxy(servername.connect("Tasks"));
	TaskQueuePrx	tasks = TaskQueuePrx::checkedCast(base);

	// get information about the task
	TaskInfo        info = tasks->info(partptr->taskid());
	if (TskCOMPLETE != info.state) {
		throw std::runtime_error("task not completed");
	}

	// get an interfaces for Images
	Ice::ObjectPrx  imagebase
		= ic->stringToProxy(servername.connect("Images"));
	ImagesPrx       images = ImagesPrx::checkedCast(imagebase);

	// get an interface for that particular image
	ImagePrx        image = images->getImage(info.filename);
	ImageFile       imagefile = image->file();

	// convert the image file to an ImagePtr
	astro::image::ImagePtr  imageptr = convertfile(imagefile);

	// add the project name to the metadata of the image
	imageptr->setMetadata(astro::io::FITSKeywords::meta("PROJECT",
		project.name()));

	// add the image to the repository
	long	repoid = repository->save(imageptr);

	// store the repository id in the project description
	astro::config::ProjectConfigurationPtr	projects
		= astro::config::ProjectConfiguration::get(config);
	projects->partrepo(project.name(), partptr->partno(), repoid);
	
	return EXIT_SUCCESS;
}

/**
 * \brief implementation of the image command, global version
 */
int	command_image(const astro::project::Project& project,
		const std::list<long> partnos) {
	for (auto partnoptr = partnos.begin(); partnoptr != partnos.end();
		partnoptr++) {
		long partno = *partnoptr;
		astro::project::PartPtr	part = project.part(partno);
		command_image(project, part);
	}
	return EXIT_SUCCESS;
}

/**
 * \brief Options for the snowtask program
 */
static struct option	longopts[] = {
{ "config",	required_argument,	NULL,		'c' }, /*  1 */
{ "debug",	no_argument,		NULL,		'd' }, /*  2 */
{ "help",	no_argument,		NULL,		'h' }, /*  3 */
{ "verbose",	no_argument,		NULL,		'v' }, /*  4 */
{ NULL,		0,			NULL,		0   }
};

/**
 * \brief Main function for the snowtask program
 */
int	main(int argc, char *argv[]) {
	CommunicatorSingleton	cs(argc, argv);
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();

	// parse command line options
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "c:dh?v",
			longopts, &longindex)))
		switch (c) {
		case 'c':
			astro::config::Configuration::set_default(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'v':
			verbose = true;
			break;
		}

	// get the command name
	if (argc <= optind) {
		throw std::runtime_error("command name missing");
	}
	std::string	command = argv[optind++];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "working on command %s",
		command.c_str());
	if (command == "help") {
		return command_help(argv[0]);
	}

	// next argument must be a project name
	if (argc <= optind) {
		throw std::runtime_error("project name missing");
	}
	std::string	projectname(argv[optind++]);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "working on project %s",
		projectname.c_str());
	if (argc <= optind) {
		throw std::runtime_error("no part numbers");
	}

	// the the Project
	astro::config::ConfigurationPtr	config
		= astro::config::Configuration::get();
	astro::config::ProjectConfigurationPtr	projects
		= astro::config::ProjectConfiguration::get(config);
	astro::project::Project	project = projects->project(projectname);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "project has %d parts, repo %s",
		project.parts.size(), project.repository().c_str());

	std::list<long>	partnos;
	while (optind < argc) {
		// get the next part
		partnos.push_back(std::stol(argv[optind++]));
	}

	if (command == "submit") {
		return command_submit(project, partnos);
	}
	if (command == "image") {
		return command_image(project, partnos);
	}

	throw std::runtime_error("unknown command");
}

} // namespace snowtask
} // namespace app
} // namespace snowtar

int	main(int argc, char *argv[]) {
	return astro::main_function<snowstar::app::snowproject::main>(argc, argv);
}
