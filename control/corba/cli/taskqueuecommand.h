/*
 * taskqueuecommand.h -- task queue command existence
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _taskqueuecommand_h
#define _taskqueuecommand_h

#include <clicommand.h>
#include <tasks.hh>

namespace astro {
namespace cli {

class taskqueuecommand : public clicommand {
	void	start(Astro::TaskQueue_var& taskqueue);
	void	stop(Astro::TaskQueue_var& taskqueue);
	void	state(Astro::TaskQueue_var& taskqueue);
	void	wait(Astro::TaskQueue_var& taskqueue);
public:
	taskqueuecommand(commandfactory& factory)
		: clicommand(factory, "taskqueue") { }
	~taskqueuecommand() { }
	virtual void	operator()(const std::string& command,
				const std::vector<std::string>& arguments);
	virtual std::string	summary() const;
	virtual std::string	help() const;
};

} // namespace cli
} // namespace astro

#endif /* _taskqueuecommand_h */
