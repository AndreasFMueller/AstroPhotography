/*
 * snowtask.cpp -- submit a task or monitor the execution of tasks on a server
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

namespace snowstar {
namespace app {
namespace snowtask {

astro::ServerName	servername;
bool	completed = false;
bool	verbose = false;
bool	dryrun = false;
astro::camera::Exposure	exposure;
std::string	instrumentname;
std::string	filter;
double	temperature = -1;
int	repeats = 1;
std::string	project;
int	cameraIndex = 0;
int	ccdIndex = 0;
int	coolerIndex = 0;
int	filterwheelIndex = 0;
int	mountIndex = 0;

void	signal_handler(int /* sig */) {
	completed = true;
}

/**
 * \brief A monitor implementation todisplay state changes
 */
class TaskMonitorI : public TaskMonitor {
public:
	TaskMonitorI() {
		std::cout << "Date       Time         Id new state";
		std::cout << std::endl;
	}
	void	stop(const Ice::Current& /* current */) {	
		completed = true;
	}
	void	update(const TaskMonitorInfo& info,
		const Ice::Current& /* current */) {
		time_t	t = converttime(info.timeago);
		std::cout << astro::timeformat("%Y-%m-%d %H:%M:%S", t);
		std::cout << astro::stringprintf(" %6d %s", info.taskid,
			state2string(info.newstate).c_str());
		std::cout << std::endl;
	}
};

/**
 * \brief Implementation of the monitor command
 */
int	command_monitor(TaskQueuePrx tasks) {
	// create a monitor callback
	Ice::ObjectPtr	callback = new TaskMonitorI();

	// register the callback with the server
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	CallbackAdapter	adapter(ic);
	Ice::Identity	ident = adapter.add(callback);
	tasks->ice_getConnection()->setAdapter(adapter.adapter());
	tasks->registerMonitor(ident);

	// wait for the termination
	signal(SIGINT, signal_handler);
	while (!completed) {
		sleep(1);
	}

	// unregister callback before exiting
	tasks->unregisterMonitor(ident);

	// that's it
	return EXIT_SUCCESS;
}

std::string	when(double timeago) {
	time_t	t = converttime(timeago);
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "timeago = %.0f, %24.24s", timeago,
	//	ctime(&t));
	struct tm	*tp = localtime(&t);
	char	buffer[128];
	if (timeago > 86400 * 365) {
		strftime(buffer, sizeof(buffer), "%y %b", tp);
	}
	if (timeago > 86400) {
		strftime(buffer, sizeof(buffer), "%b %d", tp);
	}
	if (timeago <= 86400) {
		strftime(buffer, sizeof(buffer), "%H:%M:%S", tp);
	}
	return std::string(buffer);
}

/**
 * \brief Implementation of the list command
 */
int	common_list(TaskQueuePrx tasks, const std::set<int> ids) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "listing %d tasks", ids.size());
	std::set<int>::const_iterator	i;
	std::cout << "task S size      bin  time  temp purpose ";
	if (verbose) {
		std::cout << "filter   ";
	}
	std::cout << "when     instrument ";
	if (verbose) {
		std::cout << astro::stringprintf("%-16.16s",
			"project");
	}
	std::cout << "info" << std::endl;
	for (auto ptr = ids.begin(); ptr != ids.end(); ptr++) {
		TaskParameters	parameters = tasks->parameters(*ptr);
		ImageSize	size = parameters.exp.frame.size;
		TaskInfo	info = tasks->info(*ptr);
		std::cout << astro::stringprintf("%4d ", info.taskid);
		switch (info.state) {
		case TskPENDING:
			std::cout << "P";
			break;
		case TskEXECUTING:
			std::cout << "E";
			break;
		case TskFAILED:
			std::cout << "F";
			break;
		case TskCANCELLED:
			std::cout << "X";
			break;
		case TskCOMPLETE:
			std::cout << "C";
			size = info.frame.size;
			break;
		}
		std::string	s = astro::stringprintf("%dx%d",
					size.width, size.height);
		std::cout << astro::stringprintf(" %-9.9s %1dx%1d ",
			s.c_str(),
			parameters.exp.mode.x, parameters.exp.mode.y);
		int	l = floor(log10(parameters.exp.exposuretime));
		if (l < 0) { l = 0; }
		l = 3 - l;
		if (l < 0) { l = 0; }
		std::cout << astro::stringprintf("%5.*f", l,
				parameters.exp.exposuretime);
		if (parameters.ccdtemperature < 10) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "temperature %f",
				parameters.ccdtemperature);
			std::cout << "      ";
		} else {
			std::cout << astro::stringprintf(" %5.1f",
				parameters.ccdtemperature - 273.15);
		}
		std::cout << astro::stringprintf(" %-7.7s",
			astro::camera::Exposure::purpose2string(
				convert(parameters.exp.purpose)).c_str());
		if (verbose) {
			std::cout << astro::stringprintf(" %-8.8s",
				parameters.filter.c_str());
		}
		std::cout << astro::stringprintf(" %-8.8s ",
			when(info.lastchange).c_str());
		std::cout << astro::stringprintf("%-10.10s ",
				parameters.instrument.c_str());
		if (verbose) {
			std::cout << astro::stringprintf("%-16.16s",
				parameters.project.c_str());
		}
		switch (info.state) {
		case TskPENDING:
		case TskEXECUTING:
			break;
		case TskFAILED:
		case TskCANCELLED:
			std::cout << info.cause;
			break;
		case TskCOMPLETE:
			std::cout << info.filename;
			break;
		}
		std::cout << std::endl;
	}
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the list command
 */
int	command_list(TaskQueuePrx tasks, const std::string& statestring) {
	TaskState	state = string2taskstate(statestring);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for %s tasks",
		statestring.c_str());

	// request all the task ids of this type
	std::set<int>	ids;
	taskidsequence	tasksequence = tasks->tasklist(state);
	std::copy(tasksequence.begin(), tasksequence.end(),
		std::inserter(ids, ids.begin()));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d tasks of state %s",
		ids.size(), statestring.c_str());

	// list all tasks in the set
	return common_list(tasks, ids);
}

/**
 * \brief Implementation of list command with no arguments
 *
 * The list command with no arguments retrieves all tasks from the server
 */
int	command_list(TaskQueuePrx tasks) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for tasks of all states");
	std::set<int>	ids;
	{
		taskidsequence	result = tasks->tasklist(TskPENDING);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d pending tasks",
			result.size());
		std::copy(result.begin(), result.end(),
			std::inserter(ids, ids.begin()));
	}
	{
		taskidsequence	result = tasks->tasklist(TskEXECUTING);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d executing tasks",
			result.size());
		std::copy(result.begin(), result.end(),
			std::inserter(ids, ids.begin()));
	}
	{
		taskidsequence	result = tasks->tasklist(TskFAILED);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d failed tasks",
			result.size());
		std::copy(result.begin(), result.end(),
			std::inserter(ids, ids.begin()));
	}
	{
		taskidsequence	result = tasks->tasklist(TskCANCELLED);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d cancelled tasks",
			result.size());
		std::copy(result.begin(), result.end(),
			std::inserter(ids, ids.begin()));
	}
	{
		taskidsequence	result = tasks->tasklist(TskCOMPLETE);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d completed tasks",
			result.size());
		std::copy(result.begin(), result.end(),
			std::inserter(ids, ids.begin()));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d ids total", ids.size());
	return common_list(tasks, ids);
}

/**
 * \brief Implementation of the start command
 */
int	command_start(TaskQueuePrx tasks) {
	try {
		tasks->start();
		return EXIT_SUCCESS;
	} catch (const BadState& x) {
		std::cerr << "bad state: " << x.cause << std::endl;
		return EXIT_FAILURE;
	}
}

/**
 * \brief Implementation of the stop command
 */
int	command_stop(TaskQueuePrx tasks) {
	try {
		tasks->stop();
		return EXIT_SUCCESS;
	} catch (const BadState& x) {
		std::cerr << "bad state: " << x.cause << std::endl;
		return EXIT_FAILURE;
	}
}

/**
 * \brief Implementation of the state command
 */
int	command_state(TaskQueuePrx tasks) {
	QueueState	queuestate = tasks->state();
	std::cout << state2string(queuestate) << std::endl;
	return EXIT_SUCCESS;
}

class TaskRemover {
	TaskQueuePrx&	_tasks;
public:
	TaskRemover(TaskQueuePrx& tasks) : _tasks(tasks) { }
	void	operator()(int id);
};

void	TaskRemover::operator()(int id) {
	if (dryrun) {
		std::cout << "task " << id;
		std::cout << " not removed (dry run)";
		std::cout << std::endl;
	} else {
		try {
			_tasks->remove(id);
		} catch (std::exception& x) {
			std::cerr << "cannot remove task " << id << ": ";
			std::cerr << x.what() << std::endl;
		}
	}
}

class TaskCanceller {
	TaskQueuePrx&	_tasks;
public:
	TaskCanceller(TaskQueuePrx& tasks) : _tasks(tasks) { }
	void	operator()(int id);
};

void	TaskCanceller::operator()(int id) {
	if (dryrun) {
		std::cout << "task " << id;
		std::cout << " not cancelled (dry run)";
		std::cout << std::endl;
	} else {
		try {
			_tasks->cancel(id);
		} catch (std::exception& x) {
			std::cerr << "cannot cancel task " << id << ": ";
			std::cerr << x.what() << std::endl;
		}
	}
}

/**
 * \brief Implementation of the remove command
 */
int	command_remove(TaskQueuePrx tasks, std::list<int> ids) {
	std::for_each(ids.begin(), ids.end(), TaskRemover(tasks));
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the cancel command
 */
int	command_cancel(TaskQueuePrx tasks, std::list<int> ids) {
	std::for_each(ids.begin(), ids.end(), TaskCanceller(tasks));
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the submit command
 */
int	command_submit(TaskQueuePrx tasks, InstrumentsPrx /* instruments */) {
	// get the configuration
	astro::config::ConfigurationPtr	config
		= astro::config::Configuration::get();

	// prepare the parameters 
	TaskParameters	parameters;
	parameters.project = project;
	parameters.instrument = instrumentname;

	// copy the device information
	parameters.cameraIndex = cameraIndex;
	parameters.ccdIndex = ccdIndex;
	parameters.coolerIndex = coolerIndex;
	parameters.ccdtemperature = temperature;
	parameters.filterwheelIndex = filterwheelIndex;
	parameters.filter = filter;
	parameters.mountIndex = mountIndex;

	// exposure parameters
	parameters.exp = convert(exposure);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure: %s",
		exposure.toString().c_str());

	// everything is ready now, submit the task
	try {
		for (int counter = 0; counter < repeats; counter++) {
			int	taskid = tasks->submit(parameters);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "submitted new task %d",
				taskid);
		}
	} catch (const BadParameter& x) {
		std::cerr << "bad parameter: " << x.cause << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the image command
 */
int	command_image(TaskQueuePrx tasks, int id, const std::string& filename) {
	// check whether the task really is completed
	TaskInfo	info = tasks->info(id);
	if (TskCOMPLETE != info.state) {
		throw std::runtime_error("task not completed");
	}

	// get an interfaces for Images
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
        Ice::ObjectPrx  base = ic->stringToProxy(servername.connect("Images"));
	ImagesPrx	images = ImagesPrx::checkedCast(base);

	// get an interface for that particular image
	ImagePrx	image = images->getImage(info.filename);
	ImageBuffer	imagefile = image->file(ImageEncodingFITS);

	// write the image data into a file
	int fd = open(filename.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0666);
	if (fd < 0) {
		throw std::runtime_error("cannot create file");
	}
	size_t	bytes = write(fd, imagefile.data.data(), imagefile.data.size());
	if (imagefile.data.size() != bytes) {
		std::string	msg = astro::stringprintf(
			"cannot write data: %s", strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot write data");
		close(fd);
		throw std::runtime_error(msg);
	}
	close(fd);
	return EXIT_SUCCESS;
}

/**
 * \brief Command to save an image in the remote repo
 */
int	command_remoterepo(TaskQueuePrx tasks, int id,
		const std::string& reponame) {
	TaskPrx	task = tasks->getTask(id);
	task->imageToRepo(reponame);
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the repository command
 */
int	command_repository(TaskQueuePrx tasks, int id,
		const std::string& reponame) {
	
	// check whether the task really is completed
	TaskInfo	info = tasks->info(id);
	if (TskCOMPLETE != info.state) {
		throw std::runtime_error("task not completed");
	}

	// get an interfaces for Images
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
        Ice::ObjectPrx  base = ic->stringToProxy(servername.connect("Images"));
	ImagesPrx	images = ImagesPrx::checkedCast(base);

	// get an interface for that particular image
	ImagePrx	image = images->getImage(info.filename);
	ImageBuffer	imagefile = image->file(ImageEncodingFITS);

	// convert the image file to an ImagePtr
	astro::image::ImagePtr	imageptr = convertimage(imagefile);

	// get the image repository
	astro::config::ConfigurationPtr	config
		= astro::config::Configuration::get();
	astro::config::ImageRepoConfigurationPtr	imagerepos
		= astro::config::ImageRepoConfiguration::get(config);
	astro::project::ImageRepoPtr    repo = imagerepos->repo(reponame);
	repo->save(imageptr);

	// that's it, return ok
	return EXIT_SUCCESS;
}

/**
 * \brief Usage function for the snowtask program
 */
static void	usage(const char *progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] help" << std::endl;
	std::cout << p << " [ options ] <service> help" << std::endl;
	std::cout << p << " [ options ] <service> monitor" << std::endl;
	std::cout << p << " [ options ] <service> list [ state ]" << std::endl;
	std::cout << p << " [ options ] <service> start" << std::endl;
	std::cout << p << " [ options ] <service> stop" << std::endl;
	std::cout << p << " [ options ] <service> state" << std::endl;
	std::cout << p << " [ options ] <service> cancel <id> ..." << std::endl;
	std::cout << p << " [ options ] <service> remove <id> ..." << std::endl;
	std::cout << p << " [ options ] <service> submit" << std::endl;
	std::cout << p << " [ options ] <service> project <projectname> <partno>" << std::endl;
	std::cout << p << " [ options ] <service> image <id> <filename>" << std::endl;
	std::cout << p << " [ options ] <service> remote <id> <imagerepo>" << std::endl;
	std::cout << std::endl;
	std::cout << "possible task states:" << std::endl;
	std::cout << "    pending    " << std::endl;
	std::cout << "    executing  " << std::endl;
	std::cout << "    failed     " << std::endl;
	std::cout << "    cancelled  " << std::endl;
	std::cout << "    completed  " << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -b,--binning=XxY   select XxY binning (default 1x1)"
		<< std::endl;
	std::cout << " -c,--config=<cfg>  use configuration from a cfg"
		<< std::endl;
	std::cout << " -d,--debug         increase debug level" << std::endl;
	std::cout << " -e,--exposure=t    set exposure time to t" << std::endl;
	std::cout << " -f,--filter=f      use filter named <f>" << std::endl;
	std::cout << " -h,--help          show this help and exit" << std::endl;
	std::cout << " -i,--instrument=i  use instrument named <i>"
		<< std::endl;
	std::cout << " -n,--dryrun        suppress actions that would change "
		"the queue" << std::endl;
	std::cout << " -p,--purpose=p     expose with purpose <p>" << std::endl;
	std::cout << " -r,--rectangle=r   exposre rectangle <r>" << std::endl;
	std::cout << " -s,--server<srv>   connect to the queue on <srv>"
		<< std::endl;
	std::cout << " -t,--temperature=t cool chip to temperature t"
		<< std::endl;
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
 * \brief Options for the snowtask program
 */
static struct option	longopts[] = {
{ "binning",	required_argument,	NULL,		'b' }, /*  0 */
{ "config",	required_argument,	NULL,		'c' }, /*  1 */
{ "debug",	no_argument,		NULL,		'd' }, /*  2 */
{ "dryrun",	no_argument,		NULL,		'n' }, /*  3 */
{ "exposure",	required_argument,	NULL,		'e' }, /*  4 */
{ "filter",	required_argument,	NULL,		'F' }, /*  5 */
{ "frame",	required_argument,	NULL,		'f' }, /*  6 */
{ "help",	no_argument,		NULL,		'h' }, /*  7 */
{ "instrument",	required_argument,	NULL,		'i' }, /*  8 */
{ "purpose",	required_argument,	NULL,		'p' }, /*  9 */
{ "project",	required_argument,	NULL,		'P' }, /* 10 */
{ "repeat",	required_argument,	NULL,		'r' }, /* 11 */
{ "temperature",required_argument,	NULL,		't' }, /* 13 */
{ "verbose",	no_argument,		NULL,		'v' }, /* 14 */
{ NULL,		0,			NULL,		0   }
};

/**
 * \brief Main function for the snowtask program
 */
int	main(int argc, char *argv[]) {
	debug_set_ident("snowtask");
	CommunicatorSingleton	cs(argc, argv);
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	InstrumentsPrx	instruments;

	// parse command line options
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "b:c:de:F:f:h?i:p:P:r:t:v",
			longopts, &longindex)))
		switch (c) {
		case 'b':
			exposure.mode(astro::image::Binning(optarg));
			break;
		case 'c':
			astro::config::Configuration::set_default(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'e':
			exposure.exposuretime(std::stod(optarg));
			break;
		case 'F':
			filter = optarg;
			break;
		case 'f':
			exposure.frame(astro::image::ImageRectangle(optarg));
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'i':
			instrumentname = optarg;
			{
				Ice::ObjectPrx  base = ic->stringToProxy(
					servername.connect("Instruments"));
				instruments = InstrumentsPrx::checkedCast(base);
			}
			break;
		case 'n':
			dryrun = true;
			break;
		case 'p':
			exposure.purpose(astro::camera::Exposure::string2purpose(optarg));
			debug(LOG_DEBUG, DEBUG_LOG, 0, "purpose: %s -> %d",
				optarg, exposure.purpose());
			break;
		case 'P':
			project = std::string(optarg);
			break;
		case 'r':
			repeats = std::stoi(optarg);
			break;
		case 't':
			temperature = 273.15 + std::stod(optarg);
			break;
		case 'v':
			verbose = true;
			break;
		default:
			throw std::runtime_error("unknown option");
		}

	// get the command name
	if (argc <= optind) {
		throw std::runtime_error("server or command name missing");
	}
	std::string	command = argv[optind++];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "working on command %s",
		command.c_str());
	if (command == "help") {
		return command_help(argv[0]);
	}

	// if this is not the help command, then the first string is the
	// server name
	servername = astro::ServerName(command);
	if (argc <= optind) {
		throw std::runtime_error("command missing");
	}
	command = argv[optind++];
	if (command == "help") {
		return command_help(argv[0]);
	}

	// get the Tasks interface
        Ice::ObjectPrx  base = ic->stringToProxy(servername.connect("Tasks"));
	TaskQueuePrx	tasks = TaskQueuePrx::checkedCast(base);

	// the monitor command does not need any other 
	if (command == "monitor") {
		return command_monitor(tasks);
	}
	if (command == "start") {
		return command_start(tasks);
	}
	if (command == "stop") {
		return command_stop(tasks);
	}
	if (command == "state") {
		return command_state(tasks);
	}
	if (command == "list") {
		if (optind >= argc) {
			return command_list(tasks);
		}
		std::string	statestring = argv[optind++];
		return command_list(tasks, statestring);
	}
	if (command == "remove") {
		std::list<int>	ids;
		while (optind < argc) {
			ids.push_back(atoi(argv[optind++]));
		}
		return command_remove(tasks, ids);
	}
	if (command == "cancel") {
		std::list<int>	ids;
		while (optind < argc) {
			ids.push_back(atoi(argv[optind++]));
		}
		return command_cancel(tasks, ids);
	}
	if (command == "submit") {
		return command_submit(tasks, instruments);
	}
	if (command == "image") {
		if (argc <= optind) {
			throw std::runtime_error("no id argument specified");
		}
		int	id = std::stoi(argv[optind++]);
		if (argc <= optind) {
			throw std::runtime_error("no image file name");
		}
		std::string	filename = argv[optind++];
		return command_image(tasks, id, filename);
	}
	if (command == "remote") {
		if (argc <= optind) {
			throw std::runtime_error("no id argument specified");
		}
		int	id = std::stoi(argv[optind++]);
		if (argc <= optind) {
			throw std::runtime_error("no image file name");
		}
		std::string	reponame = argv[optind++];
		return command_remoterepo(tasks, id, reponame);
	}
	if (command == "repository") {
		if (argc <= optind) {
			throw std::runtime_error("no id argument specified");
		}
		int	id = std::stoi(argv[optind++]);
		if (argc <= optind) {
			throw std::runtime_error("no repository name");
		}
		std::string	reponame = argv[optind++];
		return command_repository(tasks, id, reponame);
	}

	std::cerr << "unknown command: " << command << std::endl;
	return EXIT_FAILURE;
}

} // namespace snowtask
} // namespace app
} // namespace snowtar

int	main(int argc, char *argv[]) {
	int	rc = astro::main_function<snowstar::app::snowtask::main>(argc,
			argv);
	snowstar::CommunicatorSingleton::release();
	return rc;
}
