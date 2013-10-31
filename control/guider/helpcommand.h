/*
 * helpcommand.h -- help related commands
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _helpcommand_h
#define _helpcommand_h

#include <clicommand.h>

namespace astro {
namespace cli {

class helpcommand : public clicommand {
public:
	helpcommand(commandfactory& factory) : clicommand(factory, std::string("help")) { }
	virtual void	operator()(const std::string& command,
				const std::vector<std::string>& arguments);
	virtual std::string	summary() const;
	virtual std::string	help() const;
};

} // namespace cli
} // namespace astro

#endif /* _helpcommand_h */
