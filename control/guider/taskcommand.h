/*
 * taskcommand.h -- command class for task command
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _taskcommand_h
#define _taskcommand_h

#include <clicommand.h>

namespace astro {
namespace cli {

class taskcommand : public clicommand {
	void	info(int taskid);
public:
	taskcommand(commandfactory& factory) : clicommand(factory, "task") { }
	~taskcommand() { }
	virtual void	operator()(const std::string& command,
			const std::vector<std::string>& arguments);
	virtual std::string	summary() const;
	virtual std::string	help() const;
};

} // namespace cli
} // namespace astro

#endif /* _taskcommand_h */
