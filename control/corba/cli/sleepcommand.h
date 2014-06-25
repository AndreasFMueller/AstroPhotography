/*
 * sleepcommand.h -- sleep command
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _sleepcommand_h
#define _sleepcommand_h

#include <clicommand.h>

namespace astro {
namespace cli {

class sleepcommand : public clicommand {
public:
	sleepcommand(commandfactory& factory)
		: clicommand(factory, std::string("sleep")) { }

	virtual void	operator()(const std::string& commandname,
		const std::vector<std::string>& arguments);
	virtual std::string	help() const;
	virtual std::string	summary() const;
};

} // namespace astro
} // namespace cli

#endif /* _sleepcommand_h */
