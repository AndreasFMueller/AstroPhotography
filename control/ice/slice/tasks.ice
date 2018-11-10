//
// tasks.ice -- SLICE for the task interface
//
// (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
//
#include <camera.ice>
#include <Ice/Identity.ice>

module snowstar {
	/**
	 * \brief Task Parameters
	 */
	struct TaskParameters {
		// instrument
		string	instrument;

		// camera
		int	cameraIndex;
		int	ccdIndex;

		// cooler stuff
		int	coolerIndex;
		float	ccdtemperature;

		// filterwheel parameters
		int	filterwheelIndex;
		string	filter;

		// information about the mount
		int	mountIndex;

		// information about the focus
		int	focuserIndex;

		// project
		string	project;
		string	repodb;
		string	repository;

		// exposure stuff
		Exposure	exp;
	};

	/**
	 * \brief States a task can be in
	 *
	 * When a task is created, it is in the pending state. As soon as
	 * all devices are free, e.g. are not used by any other device,
	 * then it can be started, after which point it is in the executing
	 * state. If anything happens during execution of the task goes into
	 * the failed state. A task can also be cancelled, after which it
	 * is in the cancelled state.
	 */
	enum TaskState	{
		TskPENDING,
		TskEXECUTING,
		TskFAILED,
		TskCANCELLED,
		TskCOMPLETE
	};

	/**
	 * \brief Information about a task
	 *
	 * The TaskParameters structure only contains information abut
	 * the task parameters, the information needed for launching a
	 * task. Information collected while the task is running is not
	 * included. This additional information is returned in a TaskInfo
	 * structure
	 */
	struct TaskInfo {
		// the task id. This is slightly redundant, as we usually
		// request tasks by id, but we include it nevertheless
		int		taskid;
		// the current state of the task, and when it last changed
		TaskState	state;
		double		lastchange;
		string		cause;
		// where the produced image is storead
		string		filename;
		ImageRectangle	frame;
		// names for the devices
		string	camera;
		string	ccd;
		string	cooler;
		string	filterwheel;
		string	mount;
		string	focuser;
	};

	/**
 	 * \brief Task object
	 *
	 * Tasks can manipulated using this interface.
	 */
	interface Task {
		TaskState	state();
		TaskParameters	parameters();
		TaskInfo	info();
		string		imagename();
		Image*		getImage() throws NotFound;
		// save an image in a repository
		int	imageToRepo(string reponame) throws NotFound;
	};

	/**
	 * \brief Data structure sent to the task monitor
	 */
	struct TaskMonitorInfo {
		int		taskid;
		TaskState	newstate;
		double		timeago;
	};

	/**
	 * \brief Interface to monitor a task
	 */
	interface TaskMonitor extends Callback {
		void	update(TaskMonitorInfo info);
	};

	sequence<int> taskidsequence;

	enum QueueState {
		QueueIDLE,
		QueueLAUNCHING,
		QueueSTOPPING,
		QueueSTOPPED
	};
	/**
	 * \brief Task queue
	 *
	 * The task queue stores exposure tasks and executes them.
	 */
	interface TaskQueue {
		/**
		 * \brief manipulate the task queue 
		 */
		QueueState	state();

		/**
		 * \brief start the queue
		 */
		void	start() throws BadState;

		/**
		 * \brief stop the queue
		 */
		void	stop() throws BadState;

		/**
		 * \brief submit a new task
		 *
		 * This submits a new task to the queue.
		 */
		int	submit(TaskParameters params) throws BadParameter;

		/**
		 * \brief Retrieve the parameters of the queue task
	 	 */
		TaskParameters	parameters(int taskid) throws NotFound;

		/**
		 * \brief Retrieve the info of the queue task
		 */
		TaskInfo	info(int taskid) throws NotFound;

		/**
		 * \brief Cancel a task
		 */
		void	cancel(int taskid) throws BadState, NotFound;

		/**
		 * \brief Remove a task from the queue
		 */
		void	remove(int taskid) throws BadState, NotFound;

		/**
		 * \brief Resubmit a task
		 */
		void	resubmit(int taskid) throws BadState, NotFound;

		/**
		 * \brief retrieve a list of tasks
		 */
		taskidsequence	tasklist(TaskState state);
		
		/**
		 * \brief retrieve a task reference
		 */
		Task*	getTask(int taskid) throws NotFound;

		/**
		 * \brief register a callback
		 */
		void	registerMonitor(Ice::Identity taskmonitor);
		void	unregisterMonitor(Ice::Identity taskmonitor);
	};
};
