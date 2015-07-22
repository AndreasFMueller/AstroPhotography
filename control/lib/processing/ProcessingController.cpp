/*
 * ProcessingController.cpp -- controller to control the processing of a
 *                             network of interdependent processing steps
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <includes.h>

namespace astro {
namespace process {

//////////////////////////////////////////////////////////////////////
// Construction an destruction
//////////////////////////////////////////////////////////////////////
/**
 * \brief Construct a processing controller
 */
ProcessingController::ProcessingController() {
}

/**
 * \brief Destroy a processing controller
 */
ProcessingController::~ProcessingController() {
}

//////////////////////////////////////////////////////////////////////
// adding and removing steps
//////////////////////////////////////////////////////////////////////
/**
 * \brief add a processing step to the map
 */
void	ProcessingController::addstep(const std::string& name,
		ProcessingStepPtr step) {
	if (steps.find(name) != steps.end()) {
		throw std::runtime_error("duplicate processing step name");
	}
	ProcessingThreadPtr	thread = ProcessingThread::get(step);
	steps.insert(std::make_pair(name, thread));
}

/**
 * \brief Remove a processing step from the map
 */
void	ProcessingController::removestep(const std::string& name) {
	if (steps.find(name) == steps.end()) {
		throw std::runtime_error("no such processing step");
	}
	steps.erase(steps.find(name));
}

/**
 * \brief Find the name of a processing step
 */
std::string	ProcessingController::name(ProcessingStepPtr step) {
	stepmap::iterator	s = std::find_if(steps.begin(), steps.end(),
		[step](stepmap::value_type& v) {
			return (v.second->step() == step);
		}
	);
	if (s == steps.end()) {
		throw std::runtime_error("step not found");
	}
	return s->first;
}

/**
 * \brief Find a step by name
 */
ProcessingStepPtr	ProcessingController::find(const std::string& name) {
	stepmap::iterator	i = steps.find(name);
	if (i == steps.end()) {
		throw std::runtime_error("step named " + name + " not found");
	}
	return i->second->step();
}

//////////////////////////////////////////////////////////////////////
// Successors and predecessors
//////////////////////////////////////////////////////////////////////
void	ProcessingController::add_precursor(const std::string& target_name,
		const std::string& precursor_name) {
	find(target_name)->add_precursor(find(precursor_name));
}

void	ProcessingController::add_successor(const std::string& target_name,
		const std::string& successor_name) {
	find(target_name)->add_successor(find(successor_name));
}

void	ProcessingController::remove_precursor(const std::string& target_name,
		const std::string& precursor_name) {
	find(target_name)->remove_precursor(find(precursor_name));
}

void	ProcessingController::remove_successor(const std::string& target_name,
		const std::string& successor_name) {
	find(target_name)->remove_successor(find(successor_name));
}

//////////////////////////////////////////////////////////////////////
// execution
//////////////////////////////////////////////////////////////////////
/**
 * \brief Find out whether there is any thread that needs work
 */
bool	ProcessingController::haswork() {
	return std::any_of(steps.begin(), steps.end(),
		[](stepmap::value_type& v) {
			return (v.second->step()->status()
				== ProcessingStep::needswork);
		}
	);
}

/**
 * \brief Get one step that needs work
 */
ProcessingController::stepmap::iterator	ProcessingController::stepneedingwork() {
	return std::find_if(steps.begin(),
		steps.end(),
		[](stepmap::value_type& v) {
			return (v.second->step()->status()
				== ProcessingStep::needswork);
		}
	);
}

/**
 * \brief Auxiliary class for communication with threads
 */
class procpipe {
public:
	std::string	_name;
	int	fildes[2];
	procpipe(const std::string& name) : _name(name) {
		if (::pipe(fildes) < 0) {
			throw std::runtime_error(std::string("cannot create pipe: ")
				+ std::string(strerror(errno)));
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "fd pair %d-%d created",
			fildes[0], fildes[1]);
	}
	~procpipe() {
		close(fildes[0]);
		close(fildes[1]);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "fd pair %d-%d destroyed",
			fildes[0], fildes[1]);
	}
};
typedef std::shared_ptr<procpipe>	procpipeptr;

typedef std::list<procpipeptr>	pipelist;

/**
 * \brief Execute all threads that can be executed
 */
void	ProcessingController::execute(size_t nthreads) {
	if (nthreads == 0) {
		throw std::runtime_error("cannot execute with no threads");
	}
	pipelist	pipes;
	// keep working while we have 
	while (haswork() || (pipes.size() > 0)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"still has work, starting threads");

		// start threads while there is space
		while (haswork() && (pipes.size() < nthreads)) {
			stepmap::iterator	i = stepneedingwork();
			std::string	name = i->first;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "starting '%s'",
				name.c_str());
			// create a pipe
			procpipeptr	p = procpipeptr(new procpipe(name));
			pipes.push_back(p);
			i->second->run(p->fildes[1]);
		}

		// wait for any processes to terminate
		debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for threads");
		struct pollfd	pollfds[pipes.size()];
		struct pollfd	*pollp = (struct pollfd *)pollfds;
		int	n = 0;
		int	*np = &n;
		std::for_each(pipes.begin(), pipes.end(),
			[np,pollp](procpipeptr p) {
				pollp[*np].fd = p->fildes[0];
				pollp[*np].events = POLLIN;
				pollp[*np].revents = 0;
				(*np)++;
			}
		);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "polling %d file descriptors",
			n);
		
		if ((poll(pollfds, pipes.size(), -1)) < 0) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot poll pipes");
		}

		// now analyze the events returned in the pollfd array
		debug(LOG_DEBUG, DEBUG_LOG, 0, "reaping %d threads", n);
		for (int i = 0; i < n; i++) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "checking %d -> %d", i,
				pollfds[i].fd);
			if (pollfds[i].revents & POLLIN) {
				int	fd = pollfds[i].fd;
				// find the associate name
				pipelist::iterator	pp =
				std::find_if(pipes.begin(), pipes.end(),
					[fd](procpipeptr p) {
						return p->fildes[0] == fd;
					}
				);
				std::string	name = (*pp)->_name;
				debug(LOG_DEBUG, DEBUG_LOG, 0, "%s terminated",
					name.c_str());
				pipes.erase(pp);
				// wait for the thread
				steps.find(name)->second->wait();
			}
		}

		// start again
	}
}

} // namespace process
} // namespace astro
