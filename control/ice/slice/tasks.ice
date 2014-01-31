//
// tasks.ice -- SLICE for the task interface
//
// (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
//
#include <camera.ice>

module Astro {
	/**
	 * \brief Task Parameters
	 */
	struct TaskParameters {
		// camera
		string	camera;
		long	ccdid;

		// cooler stuff
		float	ccdtemperature;

		// filterwheel parameters
		string	filterwheel;
		long	filterposition;

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
	enum TaskState	{ TskPENDING, TskEXECUTING, TskFAILED,
			TskCANCELLED, TskCOMPLETED };

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
		long		taskid;
		// the current state of the task, and when it last changed
		TaskState	state;
		long		lastchange;
		string		cause;
		// where the produced image is storead
		string		filename;
		ImageRectangle	frame;
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
		Image		getImage() throws NotFound;
	};

	/**
	 * \brief Data structure sent to the task monitor
	 */
	struct TaskMonitorInfo {
		long		taskid;
		TaskState	newstate;
		double		timeago;
	};

	/**
	 * \brief Monitor for Task queues
	 */
	interface TaskMonitor {
		/**
		 * \break method called when a task changes state
		 */
		void	update(TaskMonitorInfo taskinfo);
		/**
		 * \brief called when the task queue is destroyed
		 */
		void	stop();
	};

	sequence<long> taskidsequence;

	enum QueueState { QueueIDLE, QueueLAUNCHING, QueueSTOPPING, QueueSTOPPED };
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
		void	start();

		/**
		 * \brief stop the queue
		 */
		void	stop();

		/**
		 * \brief submit a new task
		 *
		 * This submits a new task to the queue.
		 */
		long	submit(TaskParameters params) throws BadParameter;

		/**
		 * \brief Retrieve the parameters of the queue task
	 	 */
		TaskParameters	parameters(long taskid) throws NotFound;

		/**
		 * \brief Retrieve the info of the queue task
		 */
		TaskInfo	info(long taskid) throws NotFound;

		/**
		 * \brief Cancel a task
		 */
		void	cancel(long taskid) throws BadState, NotFound;

		/**
		 * \brief Remove a task from the queue
		 */
		void	remove(long taskid) throws BadState, NotFound;

		/**
		 * \brief retrieve a list of tasks
		 */
		taskidsequence	tasklist(TaskState state);
		
		/**
		 * \brief retrieve a task reference
		 */
		Task	getTask(long taskid) throws NotFound;

		/**
		 * \brief register a task monitor
		 */
		long	registerMonitor(TaskMonitor monitor);

		/**
		 * \brief unregister a task monitor
		 */
		void	unregisterMonitor(long monitorid);
	};
};
