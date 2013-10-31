/*
 * locatorcommand.h -- locator related commands
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _locatorcommand_h
#define _locatorcommand_h

#include <clicommand.h>

namespace astro {
namespace cli {

class locatorcommand : public clicommand {
public:
	locatorcommand(commandfactory& factory)
		: clicommand(factory, std::string("locator")) { }
	virtual void	operator()(const std::string& command,
			const std::vector<std::string>& arguments);
	virtual std::string	summary() const;
	virtual std::string	help() const;
};

} // namespace cli
} // namespace astro

#endif /* _locatorcommand_h */
